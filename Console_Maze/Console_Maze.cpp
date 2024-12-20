// Console_Maze.cpp : This file contains the 'main' function. Program execution begins and ends there.
// https://www.youtube.com/watch?v=xW8skO7MFYw&list=WL&index=14&t=4s 
// Copyright 2018-2022 OneLoneCoder.com
// made in Microsoft Visual Studio 2022

#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <chrono>
#include <cmath>


using namespace std;

#include <Windows.h>

// Setup
// set screen width and height of console to 156 rows for full screen by 41 columns in console settings (launch size)
// set the font to consolas size 16
// may have to adjust screen and map size

// TODO: Apply Texture Mapping
// TODO: Render the map using binary space partitioning?

int nScreenWidth = 156;  // 156 for full screen dell laptop (or 120 for smaller screen)
int nScreenHeight = 41;

int nMapHeight = 26.0f; // rows
int nMapWidth = 30.0f;  // columns

float fPlayerX = 7.0f;  // x pos
float fPlayerY = 24.0f;  // y pos
float fPlayerA = 3.0f;  // angle
float fPlayerA_deg = 0.0f;


float fFOV = 3.14159 / 4; // pi/4
float fDepth = 16.0f;     // 16.0f

float rotation_sensitivity = 1.0f;
float move_speed = 5.0f;

int main()
{
    // Create Screen Buffer of type wchar_t
    wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    // map (26 rows by 30 columns)
    wstring map;
    map += L"##############################";
    map += L"######......................##";
    map += L"######...#################..##";
    map += L"######...#################..##";
    map += L"#######.#############........#";
    map += L"###.........#########........#";
    map += L"###..........#......#........#";
    map += L"###..........................#";
    map += L"###..........#......#........#";
    map += L"###.........####....#........#";
    map += L"###.........#####..##........#";
    map += L"######...########..###########";
    map += L"######...########..###########";
    map += L"######...########..###########";
    map += L"######...########..#..#...#..#";
    map += L"######...########............#";
    map += L"######...########..#....#....#";
    map += L"#######.##################..##";
    map += L"#....#...#....############..##";
    map += L"#.............############..##";
    map += L"#....#...#....############..##";
    map += L"######...#################..##";
    map += L"#.............###########...##";
    map += L"#...........................##";
    map += L"#.............###########...##";
    map += L"##############################";


    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();

    // Game Loop
    while (1)
    {
        // find change in time (based on frame rate)
        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();

        // Controls
        // Handle CCW Rotation (WASD)
        if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
        {
            fPlayerA -= rotation_sensitivity * fElapsedTime;
        }
        if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
        {
            fPlayerA += rotation_sensitivity * fElapsedTime;
        }

        // player walking forward
        if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
        {
            fPlayerX += sinf(fPlayerA) * move_speed * fElapsedTime;
            fPlayerY += cosf(fPlayerA) * move_speed * fElapsedTime;

            // correct backwards if player hits the wall
            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
            {
                fPlayerX -= sinf(fPlayerA) * move_speed * fElapsedTime;
                fPlayerY -= cosf(fPlayerA) * move_speed * fElapsedTime;
            }
        }
        // player walking backward
        if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
        {
            fPlayerX -= sinf(fPlayerA) * move_speed * fElapsedTime;
            fPlayerY -= cosf(fPlayerA) * move_speed * fElapsedTime;

            // correct forwards if player hits the wall
            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
            {
                fPlayerX += sinf(fPlayerA) * move_speed * fElapsedTime;
                fPlayerY += cosf(fPlayerA) * move_speed * fElapsedTime;
            }
        }

        // press tab to turn around
        if (GetAsyncKeyState((unsigned short)VK_TAB) & 0x8000)
        {
            fPlayerA = fmod(fPlayerA + 3.14159f, 2.0 * 3.14159f); // flip the angle and normalize it to between 0 and 2 pi
        }


        // ray casting
        // for each column x
        for (int x = 0; x < nScreenWidth; x++)
        {
            // find the ray angle from the player and this column in rad
            float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

            float fDistanceToWall = 0;
            bool bHitWall = 0;
            bool bBoundary = false;

            // unit vector for ray in player space
            float fEyeX = sinf(fRayAngle);
            float fEyeY = cosf(fRayAngle);

            // until the ray hits the wall or 
            while (!bHitWall && fDistanceToWall < fDepth)
            {
                fDistanceToWall += 0.1f;

                // grow unit vector in desired direction from the player (current position of the ray)
                int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
                int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

                // if ray is out of bounds
                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
                {
                    bHitWall = true;    // ray hit the wall
                    fDistanceToWall = fDepth; // set distance to max depth
                }
                else // ray is in bounds
                {
                    // if the ray's current position is a wall block (#)
                    if (map[nTestY * nMapWidth + nTestX] == '#')
                    {
                        bHitWall = true; // ray hit the wall
                        vector<pair<float, float>> p; // distance, dot product

                        for (int tx = 0; tx < 2; tx++)
                        {
                            for (int ty = 0; ty < 2; ty++)
                            {
                                float vy = (float)nTestY + ty - fPlayerY;
                                float vx = (float)nTestX + tx - fPlayerX;
                                float d = sqrt(vx * vx + vy * vy);
                                float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
                                p.push_back(make_pair(d, dot));
                            }
                        }

                        // sort pairs closest to farthest
                        std::sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) {return left.first < right.first; });

                        float fBound = 0.01;
                        // hit boundary
                        if (acos(p.at(0).second) < fBound) bBoundary = true;
                        if (acos(p.at(1).second) < fBound) bBoundary = true;
                        // if (acos(p.at(2).second) < fBound) bBoundary = true;

                    }
                }
            }

            // Calculate distance to ceiling and floor
            int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall); // as distance to wall gets smaller, the ceiling gets larger (and vice versa)
            int nFloor = nScreenHeight - nCeiling;

            short nShade = ' ';
            short floorShade = ' ';

            // shade wall to ASCII character based on proximity
            if (fDistanceToWall <= fDepth / 4.0f)       nShade = 0x2588;    // close
            else if (fDistanceToWall < fDepth / 3.0f)   nShade = 0x2593;
            else if (fDistanceToWall < fDepth / 2.0f)   nShade = 0x2592;
            else if (fDistanceToWall < fDepth)          nShade = 0x2591;
            else                                        nShade = ' ';       // too far away

            // Black it out if we hit a boundary
            // if (bBoundary)      nShade = ' ';   // comment this out if it looks bad

            // for each row y in column x
            for (int y = 0; y < nScreenHeight; y++)
            {
                if (y < nCeiling)   // below ceiling
                {
                    screen[y * nScreenWidth + x] = ' ';
                }
                else if (y > nCeiling && y <= nFloor) // wall
                {
                    screen[y * nScreenWidth + x] = nShade;
                }
                else // floor
                {
                    float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
                    if (b < 0.25)               floorShade = '#';   // 0.25
                    else if (b < 0.5)           floorShade = 'x';   // 0.5
                    else if (b < 0.75)          floorShade = '.';   // 0.75 can only see this
                    else if (b < 0.9)           floorShade = '-';   // 0.9  and this at far distances
                    else                        floorShade = ' ';   // can see this when I can't see the wall
                    screen[y * nScreenWidth + x] = floorShade;
                }
            }
        }

        // fPlayerA_deg = ((fPlayerA + 5.00f) * (-18.00f)) % 360;
        fPlayerA_deg = fmod(fPlayerA * 180.0f / 3.14159f, 360.0f);
        if (fPlayerA_deg < 0) { fPlayerA_deg += 360.0f; }
        string empty = " ";
        // Display Stats
        swprintf_s(screen, 160, L"X=%4.2f, Y=%4.2f, A=%3.0d FPS=%3.1f %-119s", fPlayerX, fPlayerY, int(fPlayerA_deg), 1.0f / fElapsedTime, empty); // 40 chars
                

        // Map with Player centered at a square (size*size)
        int map_size = 5;
        int y_offset, x_offset;
        int x_pos, y_pos;

        
        // draw map
        for (int nx = -map_size; nx <= map_size; nx++)
        {
            for (int ny = -map_size; ny <= map_size; ny++)
            {
                y_offset = (int)fPlayerY + ny;
                x_offset = (int)fPlayerX + nx;
                x_pos = nx + map_size;
                y_pos = ny + map_size + 1;

                if (y_offset < 0 || y_offset >= nMapHeight || x_offset < 0 || x_offset >= nMapWidth) { screen[y_pos * nScreenWidth + x_pos] = ' '; }
                else { screen[y_pos * nScreenWidth + x_pos] = map[y_offset * nMapWidth + x_offset]; }

                if (nx == 0 && ny == 0) { screen[y_pos * nScreenWidth + x_pos] = 'P'; }
            }
        }

        

        /*
         // full size map
        for (int nx = 0; nx < nMapWidth; nx++)
        {
           for (int ny = 0; ny < nMapHeight; ny++)
            {
                screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
            }
        }
        
        
        // Sometimes the map is a little weird and you have to move based on the relative angle and not the map

        screen[((int)fPlayerY + 1) * nScreenWidth + (int)fPlayerX] = 'P';
        */

        // Write to Screen
        screen[nScreenWidth * nScreenHeight - 1] = '\0'; // Set last character to the escape character
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);

    }
    return 0;
}