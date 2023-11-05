#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <rlgl.h>

const char bandwidth_path[] = "/proc/net/dev";
const char ram_path[] = "/proc/meminfo";
const char temp_path[] = "/sys/class/thermal/thermal_zone0/temp";
const char cpuinfo_path[] = "/proc/cpuinfo";
const char lscpu_cmd[] = "lscpu > cpu.txt";

static int cpu_count = 1;
static int cpu_min = 1;
static int cpu_max = 9999;
static int cpu_avg = 5555;
static float cpu_avg_f = 1.0f;
static int cpu_temp = 0;
static float cpu_temp_f = 1.0f;
static float free_mem_f = 1.0f;
static int total_bytes = 0;
static int mbps = 0;

static Color faded_orange = (Color){255, 161, 0, 74};
static Color dark_faded_orange = (Color){255, 161, 0, 50};
static Color darkest_faded_orange = (Color){255, 161, 0, 35};
static Color dark_orange = (Color){200, 130, 0, 255};
static Color bg = (Color){30, 30, 30, 255};
static Color lines = (Color){255, 245, 245, 200};

void draw_freq(int rpm, int x, int y)
{
    static char hundreds_text[4];
    static char tens_text[4];

    int hundreds = rpm / 100;
    int tens = rpm - (hundreds * 100);

    sprintf(hundreds_text, "%d", hundreds);
    sprintf(tens_text, "%d", 100 + tens);

    int hundreds_width = MeasureText(hundreds_text, GetFontDefault().baseSize * 10);

    DrawText(hundreds_text, x + 120 - hundreds_width, y, GetFontDefault().baseSize * 10, ORANGE);
    DrawText(tens_text + 1, x + 127, y + 3, GetFontDefault().baseSize * 7, ORANGE);
    DrawText("m/sec", x + 130, y + 63, GetFontDefault().baseSize * 2, ORANGE);
}

void draw_speed(int speed, char *unit, int x, int y)
{
    static char speed_text[4];

    sprintf(speed_text, "%d", speed);

    int speed_width = MeasureText(speed_text, GetFontDefault().baseSize * 10);

    DrawText(speed_text, x + 120 - speed_width, y, GetFontDefault().baseSize * 10, ORANGE);
    DrawText(unit, x + 127, y + 5, GetFontDefault().baseSize * 3, ORANGE);
}

void draw_temp(int temp, char unit, int x, int y)
{
    static char temp_text[4];
    static char unit_text[4];

    sprintf(temp_text, "%d", temp);
    sprintf(unit_text, "`%c", unit);

    int temp_width = MeasureText(temp_text, GetFontDefault().baseSize * 5);
    DrawText(temp_text, x + 100 - temp_width, y, GetFontDefault().baseSize * 5, ORANGE);
    DrawText(unit_text, x + 110, y + 3, GetFontDefault().baseSize * 2, ORANGE);
}

void get_cpu_freq_range()
{
    system(lscpu_cmd);
    FILE *f = fopen("cpu.txt", "r");
    static char line[6000];
    int length = 0;

    if (f == NULL)
    {
        return;
    }

    do
    {
        // Taking input single character at a time
        char c = fgetc(f);

        // Checking for end of file
        if (feof(f))
            break;

        if (c == '\n')
        {
            line[length++] = '\0';
            if (strncmp("CPUmaxMHz:", line, 10) == 0)
            {
                cpu_max = atoi(line + 10);
            }
            else if (strncmp("CPUminMHz:", line, 10) == 0)
            {
                cpu_min = atoi(line + 10);
            }
            else if (strncmp("CPU(s):", line, 7) == 0)
            {
                cpu_count = atof(line + 7);
            }
            length = 0;
            continue;
        }

        if (c != ' ')
        {
            line[length++] = c;
        }

    } while (1);

    fclose(f);
}

void get_cpu_freq_avg()
{
    FILE *f = fopen(cpuinfo_path, "r");
    static char line[6000];
    int length = 0;

    if (f == NULL)
    {
        return;
    }

    int total_cpu = 0;

    do
    {
        // Taking input single character at a time
        char c = fgetc(f);

        // Checking for end of file
        if (feof(f))
            break;

        if (c == '\n')
        {
            line[length++] = '\0';

            if (strncmp("cpuMHz:", line, 7) == 0)
            {
                total_cpu += atoi(line + 7);
            }
            length = 0;
            continue;
        }

        if (c != ' ' && c != '\t')
        {
            line[length++] = c;
        }

    } while (1);

    fclose(f);

    cpu_avg = (float)total_cpu / (float)cpu_count;
    cpu_avg_f = (float)cpu_avg / (float)cpu_max;
}

void get_cpu_temp()
{
    FILE *f = fopen(temp_path, "r");
    static char line[6000];
    int length = 0;

    if (f == NULL)
    {
        return;
    }

    int total_cpu = 0;

    do
    {
        // Taking input single character at a time
        char c = fgetc(f);

        // Checking for end of file
        if (feof(f))
        {
            break;
        }

        if (c == '\n')
        {
            line[length++] = '\0';
            cpu_temp = atoi(line) / 1000 * 1.8f + 32;
            cpu_temp_f = (float)cpu_temp / 100.0f;
            length = 0;
            break;
        }

        if (c != ' ' && c != '\t')
        {
            line[length++] = c;
        }

    } while (1);

    fclose(f);
}

void get_ram_free()
{
    FILE *f = fopen(ram_path, "r");
    float mem_total = 1;
    float mem_free = 1;

    static char line[6000];
    int length = 0;

    if (f == NULL)
    {
        return;
    }

    do
    {
        // Taking input single character at a time
        char c = fgetc(f);

        // Checking for end of file
        if (feof(f))
            break;

        if (c == '\n')
        {
            line[length++] = '\0';

            if (strncmp("MemTotal:", line, 9) == 0)
            {
                mem_total = atoi(line + 9);
            }
            else if (strncmp("MemAvailable:", line, 13) == 0)
            {
                mem_free = atoi(line + 13);
            }
            length = 0;
            continue;
        }

        if (c != ' ' && c != '\t')
        {
            line[length++] = c;
        }

    } while (1);

    free_mem_f = mem_free / mem_total;

    fclose(f);
}

void get_bandwidth()
{
    FILE *f = fopen(bandwidth_path, "r");
    int last_bytes = total_bytes;

    static char line[6000];
    int length = 0;

    if (f == NULL)
    {
        return;
    }

    do
    {
        // Taking input single character at a time
        char c = fgetc(f);

        // Checking for end of file
        if (feof(f))
            break;

        if (c == '\n')
        {
            line[length++] = '\0';

            if (strncmp("enp6s0: ", line, 8) == 0)
            {
                total_bytes = atoi(line + 8);
            }
            length = 0;
            continue;
        }

        line[length++] = c;

    } while (1);

    if (last_bytes > 0)
    {
        mbps = (total_bytes - last_bytes) / 1024 / 1024;
    }

    fclose(f);
}

void draw_quad(Color color, Vector2 topLeft, Vector2 topRight, Vector2 bottomLeft, Vector2 bottomRight)
{
    rlBegin(RL_TRIANGLES);

    rlColor4ub(color.r, color.g, color.b, color.a);

    rlVertex2f(topLeft.x, topLeft.y);
    rlVertex2f(bottomLeft.x, bottomLeft.y);
    rlVertex2f(topRight.x, topRight.y);

    rlVertex2f(topRight.x, topRight.y);
    rlVertex2f(bottomLeft.x, bottomLeft.y);
    rlVertex2f(bottomRight.x, bottomRight.y);

    rlEnd();
}

int main(void)
{
    InitWindow(1360, 400, "XT Dashboard");
    SetTargetFPS(30);

    char out_temp_text[] = "CPU TEMP";
    int out_temp_width = MeasureText(out_temp_text, GetFontDefault().baseSize * 2);

    float update = 0.0f;

    get_cpu_freq_range();

    while (!WindowShouldClose())
    {
        update -= GetFrameTime();

        if (update <= 0.0f)
        {
            get_cpu_freq_avg();
            get_cpu_temp();
            get_ram_free();
            get_bandwidth();
            update = 1.0f;
        }

        BeginDrawing();
        ClearBackground(bg);
        DrawRectangle(GetScreenWidth() / 2 - 442, 30, 884, 312, BLACK);
        DrawRectangleLines(GetScreenWidth() / 2 - 442, 30, 884, 156, lines);
        DrawRectangleLines(GetScreenWidth() / 2 - 442, 186, 884, 156, lines);
        DrawRectangle(GetScreenWidth() / 2 - 320, 64, 640, 96, faded_orange);

        DrawRectangleLines(GetScreenWidth() / 2 - 675, 186, 200, 156, lines);
        DrawText(out_temp_text, GetScreenWidth() / 2 - 575 - out_temp_width / 2, 190, GetFontDefault().baseSize * 2, lines);
        DrawRectangle(GetScreenWidth() / 2 - 575 - 65, 220, 130, 110, faded_orange);
        draw_temp(cpu_temp, 'F', GetScreenWidth() / 2 - 575 - 67, 260);

        DrawRectangleLines(GetScreenWidth() / 2 + 475, 186, 200, 156, lines);

        draw_freq(cpu_avg, GetScreenWidth() / 2 - 320, 70);
        draw_speed(mbps, "MBs", GetScreenWidth() / 2 + 121, 70);

        DrawRectangle(GetScreenWidth() / 2 - 400, 230, 40, 80, faded_orange);
        DrawRectangle(GetScreenWidth() / 2 - 400, 230 + (80 - 80 * cpu_temp_f), 40, 80 * cpu_temp_f, ORANGE);
        DrawText("TEMP", GetScreenWidth() / 2 - 390, 315, GetFontDefault().baseSize * 2, ORANGE);

        Vector2 tl = (Vector2){
            GetScreenWidth() / 2 - 358,
            230};

        Vector2 tr = (Vector2){
            tl.x + 50,
            210};

        Vector2 bl = (Vector2){
            tl.x,
            310};

        Vector2 br = (Vector2){
            bl.x + 50,
            280};
        draw_quad(dark_faded_orange, tl, tr, bl, br);

        tl.y += 80 - 80 * cpu_temp_f;
        tr.y += 70 - 70 * cpu_temp_f;
        draw_quad(dark_orange, tl, tr, bl, br);

        draw_quad(darkest_faded_orange,
                  (Vector2){GetScreenWidth() / 2 - 340, 210},
                  (Vector2){GetScreenWidth() / 2 - 312, 210},
                  (Vector2){GetScreenWidth() / 2 - 400, 228},
                  (Vector2){GetScreenWidth() / 2 - 358, 228});

        DrawRectangle(GetScreenWidth() / 2 + 360, 230, 40, 80, faded_orange);
        DrawRectangle(GetScreenWidth() / 2 + 360, 230 + (80 - 80 * free_mem_f), 40, 80 * free_mem_f, ORANGE);
        DrawText("RAM", GetScreenWidth() / 2 + 350, 315, GetFontDefault().baseSize * 2, ORANGE);

        tr.x = GetScreenWidth() / 2 + 358;
        tr.y = 230;
        tl.x = tr.x - 50;
        tl.y = 210;

        bl.x = tl.x;
        bl.y = 280;

        br.x = tr.x;
        br.y = 310;
        draw_quad(dark_faded_orange, tl, tr, bl, br);

        tr.y += 80 - 80 * free_mem_f;
        tl.y += 70 - 70 * free_mem_f;
        draw_quad(dark_orange, tl, tr, bl, br);

        draw_quad(darkest_faded_orange,
                  (Vector2){GetScreenWidth() / 2 + 312, 210},
                  (Vector2){GetScreenWidth() / 2 + 340, 210},
                  (Vector2){GetScreenWidth() / 2 + 358, 228},
                  (Vector2){GetScreenWidth() / 2 + 400, 228});

        draw_quad(faded_orange,
                  (Vector2){GetScreenWidth() / 2 - 5, 210},
                  (Vector2){GetScreenWidth() / 2 + 5, 210},
                  (Vector2){GetScreenWidth() / 2 - 120, 320},
                  (Vector2){GetScreenWidth() / 2 + 120, 320});

        draw_quad(faded_orange,
                  (Vector2){GetScreenWidth() / 2 - 25, 210},
                  (Vector2){GetScreenWidth() / 2 - 10, 210},
                  (Vector2){GetScreenWidth() / 2 - 200, 300},
                  (Vector2){GetScreenWidth() / 2 - 110, 300});

        draw_quad(ORANGE,
                  (Vector2){GetScreenWidth() / 2 - 25, 210},
                  (Vector2){GetScreenWidth() / 2 - 10, 210},
                  (Vector2){GetScreenWidth() / 2 - 200 + 175 * (1.0f - cpu_avg_f), 300 - 90 * (1.0f - cpu_avg_f)},
                  (Vector2){GetScreenWidth() / 2 - 110 + 100 * (1.0f - cpu_avg_f), 300 - 90 * (1.0f - cpu_avg_f)});

        draw_quad(faded_orange,
                  (Vector2){GetScreenWidth() / 2 + 10, 210},
                  (Vector2){GetScreenWidth() / 2 + 25, 210},
                  (Vector2){GetScreenWidth() / 2 + 110, 300},
                  (Vector2){GetScreenWidth() / 2 + 200, 300});

        EndDrawing();
    }

    CloseWindow();

    system("rm cpu.txt");
}