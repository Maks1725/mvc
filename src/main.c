#include "config.h"
#include <math.h>
#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define BUFFER_SIZE 1920

static float buffer[BUFFER_SIZE] = {0};
static float ready_buffer[BUFFER_SIZE] = {0};
static int buff_ptr = 0;
static int channels = 2;

void audio_callback(void *bufferData, unsigned int frames) {
    float *in = bufferData;
    int remaining = BUFFER_SIZE - buff_ptr;
    int samples = frames * channels;
    int to_copy = samples > remaining ? remaining : samples;

    memcpy(&buffer[buff_ptr], in, to_copy * sizeof(float));
    buff_ptr += to_copy;

    if (buff_ptr >= BUFFER_SIZE) {
        memcpy(ready_buffer, buffer, BUFFER_SIZE * sizeof(float));
        buff_ptr = 0;
    }
}

int main(void) {
    char music_file[2048] = "Drag & Drop Music Here";
    Music music = LoadMusicStream("");

    int width = 800;
    int height = 400;

    float waves[amt_waves][BUFFER_SIZE];
    memset(waves, 0, sizeof(waves));

    Color colors[amt_waves];
    colors[0] = WHITE;
    for (int i = 1; i < amt_waves; ++i) {
        colors[i] = main_color;
        colors[i].a *= (float)(amt_waves - i) / amt_waves - 1;
    }
    Color bg_wave_colors[2] = {main_color, main_color};
    bg_wave_colors[0].a *= 0.5;
    bg_wave_colors[1].a *= 0.25;
    Color ui_color = main_color;
    ui_color.a *= 0.25;
    
    SetConfigFlags(FLAG_WINDOW_TRANSPARENT);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(width, height, "mvc");
    SetWindowMinSize(200, 200);
    SetWindowMaxSize(1920, 1080);
    InitAudioDevice();

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (IsFileDropped()) {
            FilePathList files = LoadDroppedFiles();
            if (files.count > 0) {
                if (IsMusicValid(music)) {
                    DetachAudioStreamProcessor(music.stream, audio_callback);
                    UnloadMusicStream(music);
                }
                TextCopy(music_file, files.paths[0]);
                music = LoadMusicStream(files.paths[0]);
                if (IsMusicValid(music)) {
                    channels = music.stream.channels;
                    SetMusicVolume(music, volume);
                    AttachAudioStreamProcessor(music.stream, audio_callback);
                    PlayMusicStream(music);
                }
            }
            UnloadDroppedFiles(files);
        }

        if (IsMusicValid(music)) {
            UpdateMusicStream(music);
        }

        if (IsKeyPressed(KEY_SPACE)) {
            if (IsMusicValid(music)) {
                if (IsMusicStreamPlaying(music)) {
                    PauseMusicStream(music);
                } else {
                    ResumeMusicStream(music);
                }
            }
        }

        if (IsKeyPressed(KEY_UP)) {
            volume += 0.1;
            volume = volume > 1.0 ? 1.0 : volume;
            if (IsMusicValid(music)) {
                SetMusicVolume(music, volume);
            }
        }

        if (IsKeyPressed(KEY_DOWN)) {
            volume -= 0.1;
            volume = volume < 0.0 ? 0.0 : volume;
            if (IsMusicValid(music)) {
                SetMusicVolume(music, volume);
            }
        }

        if (IsKeyPressed(KEY_LEFT)) {
            float position = GetMusicTimePlayed(music) - 10.0;
            position = position > 0 ? position : 0;
            SeekMusicStream(music, position);
        }

        if (IsKeyPressed(KEY_RIGHT)) {
            float position = GetMusicTimePlayed(music) + 10.0;
            position = position < GetMusicTimeLength(music) ? position : 0;
            SeekMusicStream(music, position);
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            int x = GetMouseX();
            int y = GetMouseY();
            if (y >= height - bar_size && IsMusicValid(music)) {
                float position = GetMusicTimeLength(music) * x / width;
                SeekMusicStream(music, position);
                UpdateMusicStream(music);
            }
        }

        for (int i = amt_waves - 1; i > 0; --i) {
            memcpy(waves[i], waves[i - 1], BUFFER_SIZE * sizeof(float));
        }
        memcpy(waves[0], ready_buffer, BUFFER_SIZE * sizeof(float));

        if (IsWindowResized()) {
            width = GetScreenWidth();
            height = GetScreenHeight();
        }

        BeginDrawing();
        ClearBackground(bg_color);

        int y;
        Color tmp;
        for (int x = 0; x < width; ++x) {
            for (int i = amt_waves - 1; i >= 0; --i) {
                y = (int)((waves[i][x] * 0.5 * wave_size + wave_pos) * height);
                DrawPixel(x, y + i * 2, colors[i]);
            }
            if (x % 16 > 1) {
                tmp = bg_wave_colors[x % 2];
                tmp.a *= 0.5 + fabs(waves[0][x]) * 0.5;
                y = height - abs((int)(waves[0][x] * bg_wave_size * height));
                DrawLine(x, y, x, height, tmp);
            }
        }

        if (IsCursorOnScreen()) {
            if (IsMusicValid(music)) {
                int progress = (int)(width * GetMusicTimePlayed(music) / GetMusicTimeLength(music));
                DrawRectangle(0, height - bar_size, progress, bar_size, main_color);
            }
            DrawRectangle(0, height - bar_size, width, bar_size, ui_bg_color);
            DrawText(GetFileNameWithoutExt(music_file), 2, height - bar_size + 2, 20, RAYWHITE);
        }

        EndDrawing();
    }

    return 0;
}
