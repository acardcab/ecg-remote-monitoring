// Compilar con:
// windres test.rc -O coff -o test.res
// gcc test.c tinyfiledialogs.c test.res -o ecgview.exe -I C:\raylib\include -L C:\raylib\lib -lraylib -lopengl32 -lgdi32 -lwinmm -lole32 -luuid -lcomdlg32 -mwindows

#define WIN32_LEAN_AND_MEAN
#define NOGDI     // evita conflicto con Rectangle
#define NOUSER    // evita conflicto con CloseWindow, ShowCursor, DrawText
#include <windows.h>
#include <wchar.h>

#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "tinyfiledialogs.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <direct.h>   // Para _mkdir en Windows
#include <locale.h>   // Para setlocale

#define MAX_POINTS 200000

bool saveImage = false;
bool reloadData = false;

// --- fopen compatible con UTF-8 (soporta acentos y tildes en rutas) ---
FILE* fopen_utf8(const char* filename, const char* mode) {
    wchar_t wfilename[MAX_PATH];
    wchar_t wmode[10];

    MultiByteToWideChar(CP_UTF8, 0, filename, -1, wfilename, MAX_PATH);
    MultiByteToWideChar(CP_UTF8, 0, mode, -1, wmode, 10);

    return _wfopen(wfilename, wmode);
}

// --- Verificar si archivo existe ---
bool FileExistsLocal(const char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

// --- Crear carpeta output si no existe ---
void EnsureOutputFolder() {
    struct stat st = {0};
    if (stat("output", &st) == -1) {
        _mkdir("output");
    }
}

// --- Generar nombre incremental ---
void GetNextFilename(char *outName, int maxLen) {
    int index = 1;
    do {
        snprintf(outName, maxLen, "output/ecg_output%d.png", index);
        index++;
    } while (FileExistsLocal(outName));
}

// --- Cargar datos ---
int LoadData(const char *filename, float *x, float *y,
             float *xmin, float *xmax, float *ymin, float *ymax) {
    FILE *fp = fopen_utf8(filename, "r");
    if (!fp) {
        tinyfd_messageBox("Error", "No se pudo abrir el archivo (revisa acentos en la ruta).", "ok", "error", 1);
        return 0;
    }

    char header[1024];
    if (!fgets(header, sizeof(header), fp)) {
        tinyfd_messageBox("Error", "No se pudo leer el encabezado del archivo.", "ok", "error", 1);
        fclose(fp);
        return 0;
    }

    int count = 0;
    int failedLines = 0;
    char line[256];

    while (count < MAX_POINTS && fgets(line, sizeof(line), fp)) {
        float tx, ty;
        if (sscanf(line, "%f,%f", &tx, &ty) == 2) {
            x[count] = tx;
            y[count] = ty;
            count++;
        } else {
            failedLines++;
        }
    }

    fclose(fp);

    if (count < 2) {
        char msg[256];
        snprintf(msg, sizeof(msg),
            "No se pudieron leer suficientes datos.\nDatos válidos: %d\nLíneas fallidas: %d",
            count, failedLines);
        tinyfd_messageBox("Error", msg, "ok", "error", 1);
        return 0;
    }

    *xmin = *xmax = x[0];
    *ymin = *ymax = y[0];
    for (int i = 1; i < count; i++) {
        if (x[i] < *xmin) *xmin = x[i];
        if (x[i] > *xmax) *xmax = x[i];
        if (y[i] < *ymin) *ymin = y[i];
        if (y[i] > *ymax) *ymax = y[i];
    }
    if (*xmax == *xmin) *xmax = *xmin + 1.0f;
    if (*ymax == *ymin) *ymax = *ymin + 1.0f;

    return count;
}

// Compilar con:
// windres test.rc -O coff -o test.res
// gcc test.c tinyfiledialogs.c test.res -o ecgview.exe -I C:\raylib\include -L C:\raylib\lib -lraylib -lopengl32 -lgdi32 -lwinmm -lole32 -luuid -lcomdlg32 -mwindows

#define WIN32_LEAN_AND_MEAN
#define NOGDI     // evita conflicto con Rectangle
#define NOUSER    // evita conflicto con CloseWindow, ShowCursor, DrawText
#include <windows.h>
#include <wchar.h>

#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "tinyfiledialogs.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <direct.h>   // Para _mkdir en Windows
#include <locale.h>   // Para setlocale

#define MAX_POINTS 200000

bool saveImage = false;
bool reloadData = false;

// --- fopen compatible con UTF-8 (soporta acentos y tildes en rutas) ---
FILE* fopen_utf8(const char* filename, const char* mode) {
    wchar_t wfilename[MAX_PATH];
    wchar_t wmode[10];

    MultiByteToWideChar(CP_UTF8, 0, filename, -1, wfilename, MAX_PATH);
    MultiByteToWideChar(CP_UTF8, 0, mode, -1, wmode, 10);

    return _wfopen(wfilename, wmode);
}

// --- Verificar si archivo existe ---
bool FileExistsLocal(const char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

// --- Crear carpeta output si no existe ---
void EnsureOutputFolder() {
    struct stat st = {0};
    if (stat("output", &st) == -1) {
        _mkdir("output");
    }
}

// --- Generar nombre incremental ---
void GetNextFilename(char *outName, int maxLen) {
    int index = 1;
    do {
        snprintf(outName, maxLen, "output/ecg_output%d.png", index);
        index++;
    } while (FileExistsLocal(outName));
}

// --- Cargar datos ---
int LoadData(const char *filename, float *x, float *y,
             float *xmin, float *xmax, float *ymin, float *ymax) {
    FILE *fp = fopen_utf8(filename, "r");
    if (!fp) {
        tinyfd_messageBox("Error", "No se pudo abrir el archivo (revisa acentos en la ruta).", "ok", "error", 1);
        return 0;
    }

    char header[1024];
    if (!fgets(header, sizeof(header), fp)) {
        tinyfd_messageBox("Error", "No se pudo leer el encabezado del archivo.", "ok", "error", 1);
        fclose(fp);
        return 0;
    }

    int count = 0;
    int failedLines = 0;
    char line[256];

    while (count < MAX_POINTS && fgets(line, sizeof(line), fp)) {
        float tx, ty;
        if (sscanf(line, "%f,%f", &tx, &ty) == 2) {
            x[count] = tx;
            y[count] = ty;
            count++;
        } else {
            failedLines++;
        }
    }

    fclose(fp);

    if (count < 2) {
        char msg[256];
        snprintf(msg, sizeof(msg),
            "No se pudieron leer suficientes datos.\nDatos válidos: %d\nLíneas fallidas: %d",
            count, failedLines);
        tinyfd_messageBox("Error", msg, "ok", "error", 1);
        return 0;
    }

    *xmin = *xmax = x[0];
    *ymin = *ymax = y[0];
    for (int i = 1; i < count; i++) {
        if (x[i] < *xmin) *xmin = x[i];
        if (x[i] > *xmax) *xmax = x[i];
        if (y[i] < *ymin) *ymin = y[i];
        if (y[i] > *ymax) *ymax = y[i];
    }
    if (*xmax == *xmin) *xmax = *xmin + 1.0f;
    if (*ymax == *ymin) *ymax = *ymin + 1.0f;

    return count;
}
