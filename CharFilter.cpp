#include <string>
#include <iostream>
#include <windows.h>
#include <opencv2/opencv.hpp>


struct screen_size {
    int width;
    int height;
    screen_size() : width(0), height(0) {}
    screen_size(int w, int h): width(w), height(h) {}
};


screen_size getScreenSize() {

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);

    int width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    return screen_size(width, height);
}


void innit(char* path) {

    std::string file = path;
    if (file.back() != 'e')
        file += ".exe";

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.wShowWindow = SW_SHOW;
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcess(file.c_str(), (char*)" 0 1", NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
        std::cout << "Failed to run CharFilter" << "\n";
    else
        std::cout << "Running CharFilter" << "\n";

    std::cout << "Press any key to terminate CharFilter . . . " << "\n";
    std::cin.ignore();

    TerminateProcess(pi.hProcess, 0);
    CloseHandle(pi.hProcess);
}


void begin() {

    system("title CharFilter");

    bool outline = true;
    bool show_video = false;
    int camId = 1;
    int captureDelay = 30;

    std::string input;

    std::cout << "Display outline (Y/n)";
    std::getline(std::cin, input);
    if (input == "n" || input == "N")
        outline = false;

    std::cout << "Display live feed (y/N)";
    std::getline(std::cin, input);
    if (input == "y" || input == "Y")
        show_video = true;

    cv::VideoCapture cap;
    cap.open(camId);
    if (!cap.isOpened()) {
        std::cout << "Failed to open camera" << "\n";
        return;
    }
        
    cv::Mat frame, gray, flip, canny, display;

    int canvas_length = 0;
    screen_size canvas_size;
    std::string canvas("#");
    std::string shade_col = ".:oO";

    do {
        cap.read(frame);
        if (frame.empty()) {
            std::cout << "Failed to read camera frame" << "\n";
            break;
        }

        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        cv::flip(gray, flip, 1);
        cv::Canny(flip, canny, 60, 180, 3);

        if (outline && show_video) {
            cv::addWeighted(flip, 0.7, canny, 0.3, 0.0, display);
            cv::imshow("Video", display);
        }
        else if (show_video) {
            cv::imshow("Video", flip);
        }

        if ((int)cv::waitKey(captureDelay) == 27)
            break;

        canvas_size = getScreenSize();
        int new_length = canvas_size.width * canvas_size.height;
        if (new_length != canvas_length) {
            canvas_length = new_length;
            canvas.resize(canvas_length);
        }

        int px_per_width = flip.cols / canvas_size.width;
        int px_per_height = flip.rows / canvas_size.height;
        int px_ratio = px_per_width * px_per_height;

        int px_off_width = (flip.cols % canvas_size.width) / 2;
        int px_off_height = (flip.rows % canvas_size.height) / 2;

        for (int y = 0; y < canvas_size.height; y++) {
            for (int x = 0; x < canvas_size.width; x++) {

                int color = 0;
                int edge_points = 0;

                for (int i = 0; i < px_per_height; i++) {
                    for (int j = 0; j < px_per_width; j++) {

                        int location_y = y * px_per_height + i + px_off_height;
                        int location_x = x * px_per_width + j + px_off_width;

                        color += flip.at<uchar>(location_y, location_x);

                        int edge = canny.at<uchar>(location_y, location_x);
                        if (edge > 0) {
                            edge_points += 1;
                        }
                    }
                }

                color = color / px_ratio;
                long long char_location = (long long)y * canvas_size.width + x;

                if (edge_points > 1 && outline) {
                    canvas[char_location] = '#';
                }
                else {
                    canvas[char_location] = shade_col[color / 63];
                }
            }
        }

        system("CLS");
        std::cout << canvas;
    } while (true);
}


int main(int argc, char** argv) {

    if (argc == 1)
        innit(argv[0]);
    else
        begin();

    return 0;
}
