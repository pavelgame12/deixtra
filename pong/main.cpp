//linker::system::subsystem  - Windows(/ SUBSYSTEM:WINDOWS)
//configuration::advanced::character set - not set
//linker::input::additional dependensies Msimg32.lib; Winmm.lib

#include "windows.h"
#include <vector>
#include "math.h"
#include <cstdlib>
#include <limits>

struct {
    HWND hWnd;//хэндл окна
    HDC device_context, context;// два контекста устройства (для буферизации)
    int width, height;//сюда сохраним размеры окна которое создаст программа
} window;

//cекция кода


void InitWindow()
{
    SetProcessDPIAware();
    window.hWnd = CreateWindow("edit", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0);

    RECT r;
    GetClientRect(window.hWnd, &r);
    window.device_context = GetDC(window.hWnd);//из хэндла окна достаем хэндл контекста устройства для рисования
    window.width = r.right - r.left;//определяем размеры и сохраняем
    window.height = r.bottom - r.top;
    window.context = CreateCompatibleDC(window.device_context);//второй буфер
    SelectObject(window.context, CreateCompatibleBitmap(window.device_context, window.width, window.height));//привязываем окно к контексту
    GetClientRect(window.hWnd, &r);

}

HPEN penRed;
HPEN penWhite;
HPEN penBlue;
HBRUSH brush;


struct point
{
    float x;
    float y;
    int l = 0;
};
struct edge
{
    int i1;
    int i2;
    int length;
};
std::vector<edge> edgelist;
std::vector<point> pointlist;

float mouseX;
float mouseY;

void getMouse()
{
    POINT p;
    GetCursorPos(&p);
    ScreenToClient(window.hWnd, &p);
    mouseX = p.x / (float)window.width;
    mouseY = p.y / (float)window.height;
}

void drawBack()
{
    RECT rect;
    GetClientRect(window.hWnd, &rect);
    auto blackBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(window.context, &rect, blackBrush);
    DeleteObject(blackBrush);
}

void drawPoint(float x,float y, float sz)
{
    Ellipse(window.context,
        x*window.width - sz* window.width,
        y*window.height- sz*window.width,
        x * window.width + sz* window.width,
        y * window.height + sz* window.width
    );

}

void Line(float x, float y, float x1,float y1)
{
    MoveToEx(window.context, x*window.width, y * window.height,NULL);
    LineTo(window.context, x1*window.width, y1* window.height);
}

const int INF = 999999999999999; 

// Рекурсивная функция для алгоритма Дейкстры
void dijkstraUtil(int u, std::vector<int>& dist, const std::vector<edge>& edgelist, std::vector<bool>& visited) {
    visited[u] = true; // Помечаем узел как посещенный

    // Обходим все рёбра
    for (const auto& edge : edgelist) {
        int v = -1;
        if (edge.i1 == u) {
            v = edge.i2; // Соединение с ребром
        }
        else if (edge.i2 == u) {
            v = edge.i1; // Соединение с ребром
        }

        // Если найден более короткий путь
        if (v != -1 && !visited[v]) {
            if (dist[u] + edge.length < dist[v]) {
                dist[v] = dist[u] + edge.length; // Обновляем расстояние
            }
        }
    }

    // Ищем следующий узел с минимальным расстоянием
    int nextNode = -1;
    int minDist = INF; // Используем постоянное значение INF

    for (int i = 0; i < dist.size(); ++i) {
        if (!visited[i] && dist[i] < minDist) {
            minDist = dist[i];
            nextNode = i;
        }
    }

    // Запускаем рекурсию для следующего узла
    if (nextNode != -1) {
        dijkstraUtil(nextNode, dist, edgelist, visited);
    }
}

// Основная функция Дейкстры
void dijkstra(int start, std::vector<point>& pointlist, std::vector<edge>& edgelist) {
    int n = pointlist.size();
    std::vector<int> dist(n, INF); // Массив для расстояний
    std::vector<bool> visited(n, false); // Массив посещенных вершин
    dist[start] = 0; // Расстояние до стартовой вершины

    // Рекурсивный вызов
    dijkstraUtil(start, dist, edgelist, visited);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    
    InitWindow();//здесь инициализируем все что нужно для рисования в окне

    penRed = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
    penWhite = CreatePen(PS_SOLID, 3, RGB(255, 255, 255));
    penBlue = CreatePen(PS_SOLID, 3, RGB(0, 0, 255));
    brush = CreateSolidBrush(RGB(128, 128, 128));

    bool tap = false;
    float pointsize = 14. / window.width;
    int ind1 = -1;
    int ind2 = -1;

    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        getMouse();

        drawBack();

        if (GetAsyncKeyState(VK_LBUTTON))
        {
            if(tap == false)
            {
                pointlist.push_back({ mouseX, mouseY });
                tap = true;
            }
        }
        else
        {
            tap = false;
        }

        if (GetAsyncKeyState(VK_RBUTTON))
        {
            
            if (ind1 == -1)
            {

                for (int i = 0; i < pointlist.size(); i++)
                {
                    float dx = pointlist[i].x - mouseX;
                    float dy = pointlist[i].y - mouseY;
                    float l = sqrt(dx * dx + dy * dy);
                    if (l < pointsize)
                    {
                        ind1 = i;
                        break;
                    }
                }
            }

            if (ind1 >= 0)
            {

                Line(pointlist[ind1].x, pointlist[ind1].y, mouseX, mouseY);

                if (ind2 == -1)
                {
                    for (int i = 0; i < pointlist.size(); i++)
                    {
                        float dx = pointlist[i].x - mouseX;
                        float dy = pointlist[i].y - mouseY;
                        float l = sqrt(dx * dx + dy * dy);
                        if (l < pointsize and ind1 != i)
                        {
                            ind2 = i;
                            break;
                        }
                    }
                }

            }
        }
        else
        {
            if (ind1 != -1 and ind2 != -1)
            {
                edgelist.push_back({ ind1, ind2 });
            }
            ind1 = -1;
            ind2 = -1;
        }


       
        
        SelectObject(window.context, penWhite);
        SelectObject(window.context, brush);

        for (int i = 0; i < pointlist.size(); i++)
        {
            //TextOutA(window.context, pointlist[i].x+10 * window.width, pointlist[i].y-15 * window.height, "A", 14);
            drawPoint(pointlist[i].x, pointlist[i].y, pointsize);
        }

        SelectObject(window.context, penRed);
        for (int i = 0; i < edgelist.size(); i++)
        {
            int i1 = edgelist[i].i1;
            int i2 = edgelist[i].i2;

            Line(pointlist[i1].x, pointlist[i1].y, pointlist[i2].x, pointlist[i2].y);
        }
        

        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//копируем буфер в окно
        Sleep(16);//ждем 16 милисекунд (1/количество кадров в секунду)
    }

}
