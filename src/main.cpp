// Main downloader file
// Copyright (C) 2024 polarbub

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 3 as published by
// the Free Software Foundation

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <curl/curl.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <stdlib.h>
#include <opencv2/opencv.hpp>

size_t write_data(void *ptr, size_t size, size_t nmemb, std::vector<u_int8_t> * outvec) {
    size_t realSize = size * nmemb;
    for (int i = 0; i < realSize; i++) {
        outvec->push_back(((u_int8_t*) (ptr))[i]);
        
    }
    return size * nmemb;
}

int readHttp(const char * URL, std::vector<u_int8_t>* outvec, long* responseCode) {
    CURL *curl = curl_easy_init();

    if(!curl) {
        std::cout << "couldn't init curl" << std::endl;
    }

    if(curl_easy_setopt(curl, CURLOPT_URL, URL) != CURLE_OK) {
        std::cout << "couldn't set options" << std::endl;
        goto cleanup;
    };
    
    if(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data) != CURLE_OK) {
        std::cout << "couldn't set options" << std::endl;
        goto cleanup;
    };

    if(curl_easy_setopt(curl, CURLOPT_WRITEDATA, outvec) != CURLE_OK) {
        std::cout << "couldn't set options" << std::endl;
        goto cleanup;
    };

    if(curl_easy_perform(curl) != CURLE_OK) {
        std::cout << "couldn't perform" << std::endl;
        goto cleanup;
    }

    if(curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, responseCode)) {
        std::cout << "couldn't getinfo" << std::endl;
        goto cleanup;
    }
    
    curl_easy_cleanup(curl);
    return 0;

    cleanup:
        curl_easy_cleanup(curl);
        return -1;
}

std::string u8ToString(u_int8_t in) {
    std::stringstream ss;
    ss << (int) in;
    return ss.str();
}

int main(int argc, char* argv[]) {
    if(argc < 3) {
        std::cout << "Not enough arguments\nUsage: " << argv[0] << " [park code] [out file name]" << std::endl;;
    }

    cv::Mat finalImage;
    cv::Mat columnImage;

    std::vector<u_int8_t> outvec;
    long code = 200;
    std::string baseURL = "https://www.nps.gov/maps/hfc/park-maps/" + std::string(argv[1]) + "brochure-map/TileGroup";
    // std::string baseURL = argv[1];

    std::cout << "Finding highest resolution" << std::endl;

    u_int8_t resNumber = 0;
    while(code == 200) {
        std::string URL = baseURL + "0/" + u8ToString(resNumber) + "-0-0.jpg";
        if(readHttp(URL.c_str(), &outvec, &code) == -1) {
            std::cout << "Couldn't read http while checking resolution" << std::endl;
            return 0;
        }

        if(code == 200) {
            std::cout << "Found resolution " << (int) resNumber << std::endl;
            resNumber++;
        }
    }
    resNumber--;


    std::cout << "\nDownloading Map" << std::endl;

    u_int8_t xCoord = 0;
    u_int8_t yCoord = 0;
    u_int8_t maxYCoord = 0;
    bool useMaxYCoord = false;
    
    u_int8_t tileGroup = 0;
    bool newTileGroup = false;

    code = 200;

    //FIX: Add back the check for tilegroup
    while(code == 200) {
        while(code == 200 && (yCoord <= maxYCoord || !useMaxYCoord)) {
            outvec.clear();
            std::string URL = baseURL + u8ToString(tileGroup) + "/" + u8ToString(resNumber) +"-" + u8ToString(xCoord) + "-" + u8ToString(yCoord) + ".jpg";
            if(readHttp(URL.c_str(), &outvec, &code) == -1) {
                std::cout << "Couldn't read http while downloading map" << std::endl;
                return 0;
            }
            
            std::cout << code << ": \"" << URL << std::endl;
            if(code == 200) {
                cv::Mat tempImage = cv::imdecode(outvec, cv::IMREAD_UNCHANGED);
                if (!tempImage.data) {
                    std::cout << "No Image data from downloaded image" << std::endl;
                    return 0;
                }

                if(!columnImage.data) {
                    columnImage = tempImage;
                } else {
                    cv::vconcat(columnImage, tempImage, columnImage);
                }

                if(yCoord > maxYCoord) {
                    maxYCoord = yCoord;
                }
                newTileGroup = false;
                yCoord++;
            }
        }

        if(yCoord == 0) {
            continue;
        }

        // std::cout << "Tile: " << (int) tileGroup << " X: " << (int) xCoord <<  Y: " << (int) yCoord << " Maxy: " << (int) maxYCoord << std::endl;
        if (((yCoord - 1) < maxYCoord || !useMaxYCoord) && !newTileGroup) {
            tileGroup++;
            code = 200;
            newTileGroup = true;
        } else if(maxYCoord == (yCoord - 1)) {
            std::cout << std::endl;
            if(!finalImage.data) {
                // std::cout << "Found " << maxYCoord << " rows" << std::endl;
                finalImage = columnImage;
            } else {
                cv::hconcat(finalImage, columnImage, finalImage);
            }

            columnImage.release();
            useMaxYCoord = true;
            tileGroup = 0;
            xCoord++;
            yCoord = 0;
            code = 200;
        }
    }

    if(!finalImage.data) {
        std::cout << "Final image has no data" << std::endl;
        return 0;
    }

    if(!cv::imwrite(argv[2], finalImage)) {
        std::cout << "Image write failure" << std::endl;
    }

    return 0;
}