/*
 * Copyright (C) 2020  Christian Berger
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Include the single-file, header-only middleware libcluon to create high-performance microservices
#include "cluon-complete.hpp"
// Include the OpenDLV Standard Message Set that contains messages that are usually exchanged for automotive or robotic applications
#include "opendlv-standard-message-set.hpp"

// Include the GUI and image processing header files from OpenCV
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// Library's
#include <string>
#include <sstream>
#include <ctime>
#include <algorithm>

// Color thresholds
cv::Scalar yellowLow = cv::Scalar(17, 89, 128);
cv::Scalar yellowHigh = cv::Scalar(35, 175, 216);
// cv::Scalar blueLow = cv::Scalar(109, 96, 27);
// cv::Scalar blueHigh = cv::Scalar(120, 189, 86);
cv::Scalar blueLow = cv::Scalar(70, 43, 34);
cv::Scalar blueHigh = cv::Scalar(120, 255, 255);

int hLow = 0, hHigh = 179, sLow = 0, sHigh = 255, vLow = 0, vHigh = 255;

// Constants
const double MAX_ANGLE = 0.290888;                      // Max steering angle for car
const double ANGLE_MARGIN = MAX_ANGLE * 0.05;           // Angle margin
const double TURN_VAL =  0.12316760378897237;           // Turning value found through linear regression
const int DIST_THRESHOLD = 32;                          // Threshold for distances from cone pos to car

// Vector of vectors to store points of the 'cones' in HSV filter img.
std::vector<std::vector<cv::Point>> blueContours;
std::vector<std::vector<cv::Point>> yellowContours;
cv::Point centerPoint, blueCone, yellowCone, blueConePrev, yellowConePrev;

// Variables
double groundSteeringRequest = 0.0;
double average = 0.0;
double steeringAngle = 0.0;
double correct = 0.0, total = 0.0, ourLeft = 0, hisLeft = 0, ourRight = 0, hisRight = 0, avgLeft = 0.0, avgRight = 0.0;
bool blueInFrame = false, yellowInFrame = false;
bool foundBlueConeOnce = false, foundYellowConeOnce = false, blueOnLeft = false, yellowOnLeft = false;

// Calculates the average accuracy of our steering angle
double steeringAccuracy()
{
    // Check if the steering is outside of the 50% margin
    if (!(steeringAngle < groundSteeringRequest * 0.5 || steeringAngle > groundSteeringRequest * 1.5))
    {
        correct++;
    }
    if (steeringAngle > 0) {
        ourLeft++;
    } else if (steeringAngle < 0) {
        ourRight++;
    }
    if (groundSteeringRequest > 0) {
        hisLeft++;
    } else if (groundSteeringRequest < 0) {
        hisRight++;
    }
    total++;
    average = (correct / total) * 100;
    avgLeft = (ourLeft / hisLeft) * 100;
    avgRight = (ourRight / hisRight) * 100;
    return average;
}

// Returns distance of object (from center)
double getDistance(cv::Point pos1, cv::Point pos2)
{           
    return sqrt(pow(pos2.x - pos1.x, 2) + pow(pos2.y - pos1.y, 2));
}

// 1 left, -1 for right
bool steer(std::string dir, double intensity)
{
    int a = dir == "Left" ? 1 : -1;
    switch (a)
    {
    case -1:
        // Check if we had been turning left, if so reset angle to 0
        if (steeringAngle > 0)
        {
            steeringAngle = 0;
        }
        break;
    case 1:
        // Check if we had been turning right, if so reset angle to 0
        if (steeringAngle < 0)
        {
            steeringAngle = 0;
        }
        break;
    default:
        return false;
    }
    steeringAngle = a * TURN_VAL * (1 + intensity);
    if (steeringAngle <= -MAX_ANGLE) {
        steeringAngle = -MAX_ANGLE;
    } else if (steeringAngle >= MAX_ANGLE) {
        steeringAngle = MAX_ANGLE;
    }
    return true;
}

double trackCones()
{
    // If both are in frame return 0
    if (blueInFrame && yellowInFrame) {
        steeringAngle = 0;
    } else {
        double intensity;
        // If yellow is on the left side
        // Car is turning clockwise
        if (yellowOnLeft) {
            // If only yellow in frame
            if (yellowInFrame) {
                intensity = (yellowCone.x / centerPoint.x);
                // Car is turning counterclockwise
                if (yellowCone.x > yellowConePrev.x) {
                    steer("Right", intensity);
                } else {
                    steeringAngle = 0;
                }
            }
            else {
                // This is where more logic is needed
                // TODO: This is where more logic is needed
                if (blueInFrame) {
                    intensity = (centerPoint.x / blueCone.x);
                    if (blueCone.y == blueConePrev.y) {
                        // The car has not moved
                        steeringAngle = 0;
                    } else if (blueCone.y < blueConePrev.y) {
                        // New cone targeted
                    } else {
                        // Current targeted cone moving closer
                        steer("Left", intensity);
                    }
                }
            }
        // If blue is on the left side
        // Car is turning clockwise
        } else if (blueOnLeft) {
            // If only blue in frame
            if (blueInFrame) {
                intensity = (blueCone.x / centerPoint.x);
                if (blueCone.x > blueConePrev.x) {
                    steer("Right", intensity);
                } else {
                    steeringAngle = 0;
                }
            } else {
                // Car is turning counterclockwise
                // TODO: This is where more logic is needed
                if (yellowInFrame) {
                    intensity = (centerPoint.x / yellowCone.x);
                    if (yellowCone.y == yellowConePrev.y) {
                        // The car has not moved
                        steeringAngle = 0;
                    } else if (yellowCone.y < yellowConePrev.y) {
                        // New cone targeted
                    } else {
                        // Current targeted cone moving closer
                        steer("Left", intensity);
                    }
                }
            }
        // If none are in frame return 0
        } else {
            steeringAngle = 0;
        }
    }
    // Update prev cone pos's
    blueConePrev = blueCone;
    yellowConePrev = yellowCone;
    // std::cout << steeringAngle
    //         << ";" << groundSteeringRequest
    //         << std::endl;
    steeringAccuracy();
    return steeringAngle;
}

// Method for filtering and creating rectangle around BLUE cones
bool getBlueCones(cv::Mat detectImage, cv::Mat drawImage, cv::Scalar color)
{
    blueInFrame = false;
    cv::Rect prevBox(cv::Point(0, 0), cv::Size(0, 0));
    cv::findContours(detectImage, blueContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point());
    if (blueContours.size() > 0)
    {
        blueInFrame = true;
        for (size_t i = 0; i < blueContours.size(); i++)
        {
            cv::Rect bBox = cv::boundingRect(blueContours[i]);
            // Add some restriction to rectangle size to avoid
            // duplicate 2x2 rectangles appearing on the same cone
            if (bBox.area() > 50)
            {
                // Only draw a new rect at the closest (bottom-most) cone
                if (bBox.y > prevBox.y)
                {
                    cv::rectangle(drawImage, bBox.tl(), bBox.br(), color, 2);
                    cv::putText(
                        drawImage,
                        "(" + std::to_string(bBox.x + (bBox.width / 2)) +
                            "," + std::to_string(bBox.y + (bBox.height / 2)) + ")",
                        cv::Point(bBox.x, bBox.y - 25),
                        5, 1,
                        cv::Scalar(0, 0, 255), 1);
                }
                prevBox = bBox;
            }
        }
        blueCone = cv::Point(prevBox.x + prevBox.width / 2, prevBox.y + prevBox.height / 2);
        if(!foundBlueConeOnce) {
            foundBlueConeOnce = true;
            blueConePrev = blueCone;
            if (blueCone.x < centerPoint.x) {
                blueOnLeft = true;
                yellowOnLeft = false;
            }
        }
        return true;
    }
    return false;
}

// Method for filtering and creating rectangle around YELLOW cones
bool getYellowCones(cv::Mat detectImage, cv::Mat drawImage, cv::Scalar color)
{
    yellowInFrame = false;
    cv::Rect prevBox(cv::Point(0, 0), cv::Size(0, 0));
    cv::findContours(detectImage, yellowContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point());
    if (yellowContours.size() > 0)
    {
        yellowInFrame = true;
        for (size_t i = 0; i < yellowContours.size(); i++)
        {
            cv::Rect bBox = cv::boundingRect(yellowContours[i]);
            // Add some restriction to rectangle size to avoid
            // duplicate 2x2 rectangles appearing on the same cone
            if (bBox.area() > 30)
            {
                // Only draw a new rect at the closest (bottom-most) cone
                if (bBox.y > prevBox.y)
                {
                    cv::rectangle(drawImage, bBox.tl(), bBox.br(), color, 2);
                    cv::putText(
                        drawImage,
                        "(" + std::to_string(bBox.x + (bBox.width / 2)) +
                            "," + std::to_string(bBox.y + (bBox.height / 2)) + ")",
                        cv::Point(bBox.x, bBox.y - 25),
                        5, 1,
                        cv::Scalar(0, 0, 255), 1);
                }
                prevBox = bBox;
            }
        }
        yellowCone = cv::Point(prevBox.x + prevBox.width / 2, prevBox.y + prevBox.height / 2);
        if(!foundYellowConeOnce) {
            foundYellowConeOnce = true;
            yellowConePrev = yellowCone;
            if (yellowCone.x < centerPoint.x) {
                blueOnLeft = false;
                yellowOnLeft = true;
            }
        }
        return true;
    }
    return false;
}

int32_t main(int32_t argc, char **argv)
{
    int32_t retCode{1};
    // Parse the command line parameters as we require the user to specify some mandatory information on startup.
    auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);
    if ((0 == commandlineArguments.count("cid")) ||
        (0 == commandlineArguments.count("name")) ||
        (0 == commandlineArguments.count("width")) ||
        (0 == commandlineArguments.count("height")))
    {
        std::cerr << argv[0] << " attaches to a shared memory area containing an ARGB image." << std::endl;
        std::cerr << "Usage:   " << argv[0] << " --cid=<OD4 session> --name=<name of shared memory area> [--verbose]" << std::endl;
        std::cerr << "         --cid:    CID of the OD4Session to send and receive messages" << std::endl;
        std::cerr << "         --name:   name of the shared memory area to attach" << std::endl;
        std::cerr << "         --width:  width of the frame" << std::endl;
        std::cerr << "         --height: height of the frame" << std::endl;
        std::cerr << "Example: " << argv[0] << " --cid=253 --name=img --width=640 --height=480 --verbose" << std::endl;
    }
    else
    {
        // Extract the values from the command line parameters
        const std::string NAME{commandlineArguments["name"]};
        const uint32_t WIDTH{static_cast<uint32_t>(std::stoi(commandlineArguments["width"]))};
        const uint32_t HEIGHT{static_cast<uint32_t>(std::stoi(commandlineArguments["height"]))};
        const bool VERBOSE{commandlineArguments.count("verbose") != 0};

        // Attach to the shared memory.
        std::unique_ptr<cluon::SharedMemory> sharedMemory{new cluon::SharedMemory{NAME}};
        if (sharedMemory && sharedMemory->valid())
        {
            std::clog << argv[0] << ": Attached to shared memory '" << sharedMemory->name() << " (" << sharedMemory->size() << " bytes)." << std::endl;

            // Interface to a running OpenDaVINCI session where network messages are exchanged.
            // The instance od4 allows you to send and receive messages.
            cluon::OD4Session od4{static_cast<uint16_t>(std::stoi(commandlineArguments["cid"]))};

            opendlv::proxy::GroundSteeringRequest gsr;
            std::mutex gsrMutex;
            auto onGroundSteeringRequest = [&gsr, &gsrMutex](cluon::data::Envelope &&env) {
                // The envelope data structure provide further details, such as sampleTimePoint as shown in this test case:
                // https://github.com/chrberger/libcluon/blob/master/libcluon/testsuites/TestEnvelopeConverter.cpp#L31-L40
                std::lock_guard<std::mutex> lck(gsrMutex);
                gsr = cluon::extractMessage<opendlv::proxy::GroundSteeringRequest>(std::move(env));
                // std::cout << "groundSteering = " << gsr.groundSteering() << std::endl;
            }; 

            od4.dataTrigger(opendlv::proxy::GroundSteeringRequest::ID(), onGroundSteeringRequest);

            // Crop zone for image
            cv::Rect roi(
                0,             // x pos
                HEIGHT / 2,    // y pos
                WIDTH - 1,     // rect width
                (HEIGHT / 5)); // rect height

            // OpenCV data structure to hold an image.
            cv::Mat img, imgFrame, imgBlur, imgHSV, frameHSV, frameCropped, hsvDebug;
            centerPoint = cv::Point(WIDTH / 2, roi.height);

            if (VERBOSE)
            {
                cv::namedWindow("HSV Debugger");
                cv::createTrackbar("Hue - low", "HSV Debugger", &hLow, 179);
                cv::createTrackbar("Hue - high", "HSV Debugger", &hHigh, 179);
                cv::createTrackbar("Sat - low", "HSV Debugger", &sLow, 255);
                cv::createTrackbar("Sat - high", "HSV Debugger", &sHigh, 255);
                cv::createTrackbar("Val - low", "HSV Debugger", &vLow, 255);
                cv::createTrackbar("Val - high", "HSV Debugger", &vHigh, 255);
            }

            // Endless loop; end the program by pressing Ctrl-C.
            while (od4.isRunning())
            {
                // Wait for a notification of a new frame.
                sharedMemory->wait();

                // Performance reading start
                uint64_t startFrame = cv::getTickCount();

                // Lock the shared memory.
                sharedMemory->lock();
                {
                    // Copy the pixels from the shared memory into our own data structure.
                    cv::Mat wrapped(HEIGHT, WIDTH, CV_8UC4, sharedMemory->data());
                    cv::Mat cropWrapped(HEIGHT, WIDTH, CV_8UC1, sharedMemory->data());
                    imgFrame = cropWrapped.clone();
                    img = wrapped.clone();
                }
                // TODO: Here, you can add some code to check the sampleTimePoint when the current frame was captured.
                std::pair<bool, cluon::data::TimeStamp> timestampFromImage = sharedMemory->getTimeStamp();
                std::string timestamp = std::to_string(cluon::time::toMicroseconds(timestampFromImage.second));
                std::cout << "Group 1;" << timestamp << ";" << steeringAngle << std::endl; 
                sharedMemory->unlock();

                cluon::data::TimeStamp ts = cluon::time::now();
                uint32_t seconds = ts.seconds();
                std::stringstream stream;
                std::time_t time = static_cast<time_t>(seconds);
                tm *p_time = gmtime(&time);
                stream << "Now: "
                       << p_time->tm_year + 1900 // needed to add 1900 because ctime tm_year is current year - 1900
                       << "-";
                if (p_time->tm_mon < 10)
                {
                    stream << "0";
                }
                stream << p_time->tm_mon + 1 // based on 0-11 range, +1 to correct
                       << "-";
                if (p_time->tm_mday < 10)
                {
                    stream << "0";
                }
                stream << p_time->tm_mday
                       << "T";
                if (p_time->tm_hour < 10)
                {
                    stream << "0";
                }
                stream << p_time->tm_hour + 1 // same as with the month, 0-23 hour range
                       << ":";
                if (p_time->tm_min < 10)
                {
                    stream << "0";
                }
                stream << p_time->tm_min
                       << ":";
                if (p_time->tm_sec < 10)
                {
                    stream << "0";
                }
                stream << p_time->tm_sec
                       << "Z";
                std::string date = stream.str();

                // Cropped image frame
                frameCropped = img(roi);

                // Blur the input stream
                cv::blur(frameCropped, imgBlur, cv::Size(7, 7));

                // Convert BGR -> HSV
                cv::cvtColor(imgBlur, imgHSV, cv::COLOR_BGR2HSV);

                // ----> Call 2x method here <-----
                cv::inRange(imgHSV, blueLow, blueHigh, frameHSV);
                getBlueCones(frameHSV, frameCropped, cv::Scalar(255, 0, 0));

                cv::inRange(imgHSV, yellowLow, yellowHigh, frameHSV);
                getYellowCones(frameHSV, frameCropped, cv::Scalar(0, 255, 255));
                // ----> Call 2x method here <-----

                trackCones();

                // Performance reading end
                uint64_t endFrame = cv::getTickCount();
                std::string calcSpeed = std::to_string(((endFrame - startFrame) / cv::getTickFrequency()) * 1000);

                // Average correct values converted to string
                std::string averageText = std::to_string(average);
                std::string averageLeftText = std::to_string(avgLeft);
                std::string averageRightText = std::to_string(avgRight);

                // If you want to access the latest received ground steering, don't forget to lock the mutex:
                {
                    std::lock_guard<std::mutex> lck(gsrMutex);
                    //std::cout << "main: groundSteering = " << gsr.groundSteering() << std::endl;
                    //std::cout << "group_01;" << timestamp << ";" << steeringAngle << std::endl;
                    groundSteeringRequest = gsr.groundSteering();
                }

                // Display image on your screen.
                if (VERBOSE)
                {
                    hsvDebug = imgHSV.clone();
                    cv::blur(hsvDebug, hsvDebug, cv::Size(7, 7));
                    cv::cvtColor(hsvDebug, hsvDebug, cv::COLOR_BGR2HSV);
                    cv::inRange(hsvDebug, cv::Scalar(hLow, sLow, vLow), cv::Scalar(hHigh, sHigh, vHigh), hsvDebug);
                    hLow = cv::getTrackbarPos("Hue - low", "HSV Debugger");
                    hHigh = cv::getTrackbarPos("Hue - high", "HSV Debugger");
                    sLow = cv::getTrackbarPos("Sat - low", "HSV Debugger");
                    sHigh = cv::getTrackbarPos("Sat - high", "HSV Debugger");
                    vLow = cv::getTrackbarPos("Val - low", "HSV Debugger");
                    vHigh = cv::getTrackbarPos("Val - high", "HSV Debugger");

                    cv::putText(img, date, cv::Point(25, 25), 5, 1, cv::Scalar(255, 255, 0), 1);
                    cv::putText(img, "TS: " + timestamp, cv::Point(25, 45), 5, 1, cv::Scalar(255, 255, 0), 1);
                    cv::putText(img, "Calculation Speed (ms): " + calcSpeed, cv::Point(25, 65), 5, 1, cv::Scalar(255, 255, 0), 1);
                    cv::putText(img, "Average % " + averageText, cv::Point(25, 85), 5, 1, cv::Scalar(255, 255, 0), 1);
                    cv::putText(img, "Left Turn % " + averageLeftText , cv::Point(25, 105), 5, 1, cv::Scalar(255, 255, 0), 1);
                    cv::putText(img, "Right Turn % " + averageRightText, cv::Point(25, 125), 5, 1, cv::Scalar(255, 255, 0), 1);
                    cv::imshow(sharedMemory->name().c_str(), img);
                    cv::imshow("Filter - Debug", hsvDebug);
                    // cv::imshow("Image Crop - Debug", frameCropped);
                    cv::waitKey(1);
                }
            }
        }
        retCode = 0;
    }
    return retCode;
}