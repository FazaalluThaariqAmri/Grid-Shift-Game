#include "raylib.h"
#include <cstdlib>
#include <ctime>
#include <fstream>

enum GameState { MENU, PLAYING, SETTINGS };

struct Vector2D {
    int x;
    int y;
};

int main() {
    const int screenWidth  = 1100;
    const int screenHeight = 700;
    InitWindow(screenWidth, screenHeight, "Grid Shift: Visual Linear Algebra");
    SetTargetFPS(60);

    InitAudioDevice();

    Music bgMusic = LoadMusicStream("bgm.mp3");
    Sound winSound = LoadSound("win.wav");

    GameState currentState = MENU;

    int bgVolumeInt  = 5; // Representasi 0-10 (50% default)
    int sfxVolumeInt = 5;
    float bgVolume   = bgVolumeInt  / 10.0f;
    float sfxVolume  = sfxVolumeInt / 10.0f;

    PlayMusicStream(bgMusic);
    SetMusicVolume(bgMusic, bgVolume);
    SetSoundVolume(winSound, sfxVolume);

    srand(time(0));

    Vector2D v1, v2, target;
    int ans_c1, ans_c2;
    int c1 = 0, c2 = 0;
    int level = 1;
    bool levelSelesai = false;
    bool keluarGame   = false;

    int highScore = 0;
    std::ifstream fileMasuk("highscore.txt");
    if (fileMasuk.is_open()) {
        fileMasuk >> highScore;
        fileMasuk.close();
    }

    auto generateLevel = [&]() {
        // Grid -15 s/d +15, titik ungu selalu di (0,0) / tengah grid
        do {
            v1 = {(rand() % 7) - 3, (rand() % 7) - 3};
            v2 = {(rand() % 7) - 3, (rand() % 7) - 3};
            ans_c1 = (rand() % 11) - 5;
            ans_c2 = (rand() % 11) - 5;
            target = {(ans_c1 * v1.x) + (ans_c2 * v2.x),
                      (ans_c1 * v1.y) + (ans_c2 * v2.y)};
        } while (target.x < -15 || target.x > 15 ||
                 target.y < -15 || target.y > 15 ||
                 (target.x == 0 && target.y == 0) ||
                 (v1.x == 0 && v1.y == 0) ||
                 (v2.x == 0 && v2.y == 0) ||
                 (ans_c1 == 0 && ans_c2 == 0));
        c1 = 0; c2 = 0;
        levelSelesai = false;
    };

    generateLevel();

    // --- Tombol MENU ---
    Rectangle btnPlay     = { (1100-200)/2, 250, 200, 50 };
    Rectangle btnSettings = { (1100-200)/2, 330, 200, 50 };
    Rectangle btnExit     = { (1100-200)/2, 410, 200, 50 };
    Rectangle btnReset    = { (1100-200)/2, 490, 200, 50 };

    // --- Tombol SETTINGS ---
    Rectangle btnVolMin  = { 300, 250,  50, 50 };
    Rectangle btnVolPlus = { 450, 250,  50, 50 };
    Rectangle btnSfxMin  = { 300, 350,  50, 50 };
    Rectangle btnSfxPlus = { 450, 350,  50, 50 };
    Rectangle btnBack    = { 300, 450, 200, 50 };

    // --- Tombol PLAYING ---
    Rectangle btnBackMenu = { 800, 620, 200, 40 };

    while (!WindowShouldClose() && !keluarGame) {
        UpdateMusicStream(bgMusic);
        Vector2 mousePoint = GetMousePosition();

        // Hitung posisi saat ini setiap frame
        Vector2D currentPos;
        currentPos.x = (c1 * v1.x) + (c2 * v2.x);
        currentPos.y = (c1 * v1.y) + (c2 * v2.y);

        if (currentState == MENU) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(mousePoint, btnPlay))     currentState = PLAYING;
                if (CheckCollisionPointRec(mousePoint, btnSettings)) currentState = SETTINGS;
                if (CheckCollisionPointRec(mousePoint, btnExit))     keluarGame = true;
                if (CheckCollisionPointRec(mousePoint, btnReset)) {
                    highScore = 0;
                    std::ofstream fileKeluar("highscore.txt");
                    if (fileKeluar.is_open()) {
                        fileKeluar << highScore;
                        fileKeluar.close();
                    }
                }
            }
        }
        else if (currentState == SETTINGS) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(mousePoint, btnVolMin)  && bgVolumeInt  > 0)  bgVolumeInt--;
                if (CheckCollisionPointRec(mousePoint, btnVolPlus) && bgVolumeInt  < 10) bgVolumeInt++;
                if (CheckCollisionPointRec(mousePoint, btnSfxMin)  && sfxVolumeInt > 0)  sfxVolumeInt--;
                if (CheckCollisionPointRec(mousePoint, btnSfxPlus) && sfxVolumeInt < 10) sfxVolumeInt++;
                if (CheckCollisionPointRec(mousePoint, btnBack))    currentState = MENU;
            }

            bgVolume  = bgVolumeInt  / 10.0f;
            sfxVolume = sfxVolumeInt / 10.0f;
            SetMusicVolume(bgMusic, bgVolume);
            SetSoundVolume(winSound, sfxVolume);
        }
        else if (currentState == PLAYING) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(mousePoint, btnBackMenu)) {
                    currentState = MENU;
                    level = 1;
                    generateLevel();
                }
            }

            if (!levelSelesai) {
                
                int old_c1 = c1, old_c2 = c2;

                if (IsKeyPressed(KEY_UP))    c1++;
                if (IsKeyPressed(KEY_DOWN))  c1--;
                if (IsKeyPressed(KEY_RIGHT)) c2++;
                if (IsKeyPressed(KEY_LEFT))  c2--;

                int newX = (c1 * v1.x) + (c2 * v2.x);
                int newY = (c1 * v1.y) + (c2 * v2.y);

                if (newX < -15 || newX > 15 || newY < -15 || newY > 15) {
                    c1 = old_c1; // batalkan gerakan
                    c2 = old_c2; // batas -15 s/d +15
                }
            }

            if (currentPos.x == target.x && currentPos.y == target.y) {
                if (!levelSelesai) PlaySound(winSound);
                levelSelesai = true;
                if (IsKeyPressed(KEY_ENTER)) {
                    level++;
                    if (level > highScore) {
                        highScore = level;
                        std::ofstream fileKeluar("highscore.txt");
                        if (fileKeluar.is_open()) {
                            fileKeluar << highScore;
                            fileKeluar.close();
                        }
                    }
                    generateLevel();
                }
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (currentState == MENU) {
            // Judul dan High Score di tengah layar
            const char* title = "GRID SHIFT";
            int titleWidth = MeasureText(title, 50);
            DrawText(title, (screenWidth - titleWidth) / 2, 120, 50, DARKPURPLE);

            const char* hsText = TextFormat("High Score: Level %d", highScore);
            int hsWidth = MeasureText(hsText, 20);
            DrawText(hsText, (screenWidth - hsWidth) / 2, 190, 20, DARKGRAY);

            Color colPlay     = CheckCollisionPointRec(mousePoint, btnPlay)     ? GRAY : LIGHTGRAY;
            Color colSettings = CheckCollisionPointRec(mousePoint, btnSettings) ? GRAY : LIGHTGRAY;
            Color colExit     = CheckCollisionPointRec(mousePoint, btnExit)     ? GRAY : LIGHTGRAY;

            DrawRectangleRec(btnPlay,     colPlay);
            DrawText("PLAY",     (1100 - MeasureText("PLAY",     20)) / 2, 265, 20, BLACK);

            DrawRectangleRec(btnSettings, colSettings);
            DrawText("SETTINGS", (1100 - MeasureText("SETTINGS", 20)) / 2, 345, 20, BLACK);

            DrawRectangleRec(btnExit,     colExit);
            DrawText("EXIT",     (1100 - MeasureText("EXIT",     20)) / 2, 425, 20, BLACK);

            Color colReset = CheckCollisionPointRec(mousePoint, btnReset) ? GRAY : LIGHTGRAY;
            DrawRectangleRec(btnReset, colReset);
            DrawText("RESET SCORE", (1100 - MeasureText("RESET SCORE", 20)) / 2, 505, 20, RED);
        }
        else if (currentState == SETTINGS) {
            DrawText("SETTINGS", 310, 120, 40, DARKGRAY);

            // --- Volume Musik ---
            DrawText("Volume Musik", 320, 200, 20, BLACK);

            Color colVolMin  = CheckCollisionPointRec(mousePoint, btnVolMin)  ? GRAY : LIGHTGRAY;
            Color colVolPlus = CheckCollisionPointRec(mousePoint, btnVolPlus) ? GRAY : LIGHTGRAY;
            DrawRectangleRec(btnVolMin,  colVolMin);
            DrawText("-", 320, 265, 20, BLACK);
            DrawText(TextFormat("%d0%%", bgVolumeInt), 370, 265, 20, DARKPURPLE);
            DrawRectangleRec(btnVolPlus, colVolPlus);
            DrawText("+", 470, 265, 20, BLACK);

            // --- Volume Efek Suara ---
            DrawText("Volume Efek Suara", 300, 320, 20, BLACK);

            Color colSfxMin  = CheckCollisionPointRec(mousePoint, btnSfxMin)  ? GRAY : LIGHTGRAY;
            Color colSfxPlus = CheckCollisionPointRec(mousePoint, btnSfxPlus) ? GRAY : LIGHTGRAY;
            DrawRectangleRec(btnSfxMin,  colSfxMin);
            DrawText("-", 320, 365, 20, BLACK);
            DrawText(TextFormat("%d0%%", sfxVolumeInt), 370, 365, 20, DARKPURPLE);
            DrawRectangleRec(btnSfxPlus, colSfxPlus);
            DrawText("+", 470, 365, 20, BLACK);

            Color colBack = CheckCollisionPointRec(mousePoint, btnBack) ? GRAY : LIGHTGRAY;
            DrawRectangleRec(btnBack, colBack);
            DrawText("BACK", 370, 465, 20, BLACK);
        }
        else if (currentState == PLAYING) {
            DrawText(TextFormat("LEVEL %d", level), 720, 50, 30, DARKGRAY);
            DrawText(TextFormat("Target : [%d, %d]", target.x, target.y), 720, 100, 20, RED);
            DrawText(TextFormat("Vektor 1 : [%d, %d]", v1.x, v1.y), 720, 140, 20, BLUE);
            DrawText(TextFormat("Vektor 2 : [%d, %d]", v2.x, v2.y), 720, 170, 20, GREEN);

            DrawText("KONTROL PEMAIN:", 720, 250, 20, DARKGRAY);
            DrawText(TextFormat("c1 (Naik/Turun) : %d", c1), 720, 280, 20, BLUE);
            DrawText(TextFormat("c2 (Kanan/Kiri) : %d", c2), 720, 310, 20, GREEN);
            DrawText(TextFormat("Posisi Saat Ini : [%d, %d]", currentPos.x, currentPos.y), 720, 350, 20, DARKPURPLE);

            // Grid -15 s/d +15: 30 sel, cellSize=20, area 600x600
            // originX/Y = pixel posisi koordinat (0,0) = tengah grid
            int cellSize  = 20;
            int gridRange = 15;
            int originX   = 310; // pixel X untuk koordinat 0
            int originY   = 350; // pixel Y untuk koordinat 0

            // Gambar garis grid
            for (int i = -gridRange; i <= gridRange; i++) {
                int px = originX + (i * cellSize);
                int py = originY - (i * cellSize);
                // garis vertikal
                DrawLine(px, originY - (gridRange * cellSize),
                         px, originY + (gridRange * cellSize),
                         LIGHTGRAY);
                // garis horizontal
                DrawLine(originX - (gridRange * cellSize), py,
                         originX + (gridRange * cellSize), py,
                         LIGHTGRAY);
            }

            // Gambar label sumbu 0
            DrawText("0", originX + 3, originY + 3, 12, DARKGRAY);

            // Gambar titik merah (target)
            int targetPixX = originX + (target.x * cellSize);
            int targetPixY = originY - (target.y * cellSize);
            DrawRectangle(targetPixX - 8, targetPixY - 8, 16, 16, RED);

            // Gambar titik ungu (pemain) — selalu mulai di (0,0) = tengah
            int playerPixX = originX + (currentPos.x * cellSize);
            int playerPixY = originY - (currentPos.y * cellSize);
            DrawCircle(playerPixX, playerPixY, 10, DARKPURPLE);

            if (levelSelesai) {
                DrawText("TEPAT SASARAN!", 720, 450, 25, GOLD);
                DrawText("Tekan [ENTER] untuk lanjut", 720, 490, 15, DARKGRAY);
            }

            Color colBackMenu = CheckCollisionPointRec(mousePoint, btnBackMenu) ? GRAY : LIGHTGRAY;
            DrawRectangleRec(btnBackMenu, colBackMenu);
            DrawText("BACK TO MENU", 810, 630, 15, BLACK);
        }

        EndDrawing();
    }

    UnloadSound(winSound);
    UnloadMusicStream(bgMusic);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}