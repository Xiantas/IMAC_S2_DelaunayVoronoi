#include "application_ui.h"
#include "SDL2_gfxPrimitives.h"
#include <vector>
#include <list>
#include <map>
#include <queue>
#include <algorithm>
#include <random>

#include <iostream>
#include <cstdio>

#define EPSILON 0.0001f


std::random_device rd{};
std::mt19937 generator{rd()};
std::uniform_int_distribution<int> dis{1, 0xffffff};


struct Coords {
    int x, y;

    bool operator==(const Coords& other) {
        return x == other.x and y == other.y;
    }

    bool operator!=(const Coords& other) {
        return x != other.x || y != other.y;
    }
};

struct Segment {
    Coords p1, p2;

    bool operator==(const Segment &other) {
        return (p1 == other.p1 && p2 == other.p2) || (p1 == other.p2 && p2 == other.p1);
    }
};

struct Triangle {
    Coords p1, p2, p3;
    bool complet=false;
    Uint32 color = 0;

    bool hasPoint(Coords p) {
        return p == p1 || p == p2 || p == p3;
    }

    bool hasPoints(Coords pa, Coords pb) {
        return (pa == p1 || pa == p2 || pa == p3)
            && (pb == p1 || pb == p2 || pb == p3);
    }

    Coords getOther(Coords p) {
        if (p != p1) {
            return p1;
        } else if (p != p2) {
            return p2;
        } else {
            return p3;
        }
    }

    Coords getOther(Coords pa, Coords pb) {
        if (pa != p1 && pb != p1) {
            return p1;
        } else if (pa != p2 && pb != p2) {
            return p2;
        } else {
            return p3;
        }
    }
};

struct Polygon {
    std::vector<Sint16> xs;
    std::vector<Sint16> ys;
    Uint32 color;
};

struct Application {
    int width, height;
    Coords focus{100, 100};

    bool filled = false;
    bool voronoi = false;

    std::vector<Coords> points;
    std::vector<Triangle> triangles;
    std::vector<Polygon> polygons;
};

bool compareCoords(Coords point1, Coords point2)
{
    if (point1.y == point2.y)
        return point1.x < point2.x;
    return point1.y < point2.y;
}

void drawPoints(SDL_Renderer *renderer, const std::vector<Coords> &points)
{
    for (std::size_t i = 0; i < points.size(); i++)
    {
        filledCircleRGBA(renderer, points[i].x, points[i].y, 3, 240, 240, 23, SDL_ALPHA_OPAQUE);
    }
}

void drawSegments(SDL_Renderer *renderer, const std::vector<Segment> &segments)
{
    for (std::size_t i = 0; i < segments.size(); i++)
    {
        lineRGBA(
            renderer,
            segments[i].p1.x, segments[i].p1.y,
            segments[i].p2.x, segments[i].p2.y,
            240, 240, 20, SDL_ALPHA_OPAQUE);
    }
}

void drawTrianglesFilled(SDL_Renderer *renderer, const std::vector<Triangle> &triangles) {
    for (std::size_t i = 0; i < triangles.size(); i++) {
        const Triangle& t = triangles[i];
        filledTrigonColor(
            renderer,
            t.p1.x, t.p1.y,
            t.p2.x, t.p2.y,
            t.p3.x, t.p3.y,
            t.color
        );
    }
}

void drawTriangles(SDL_Renderer *renderer, const std::vector<Triangle> &triangles) {
    for (std::size_t i = 0; i < triangles.size(); i++) {
        const Triangle& t = triangles[i];
        trigonRGBA(
            renderer,
            t.p1.x, t.p1.y,
            t.p2.x, t.p2.y,
            t.p3.x, t.p3.y,
            0, 240, 160, SDL_ALPHA_OPAQUE
        );
    }
}

void drawPoly(SDL_Renderer *renderer, const std::vector<Polygon> &polys) {
    for (std::size_t i = 0; i < polys.size(); i++) {
        const Polygon& p = polys[i];
        polygonRGBA(
            renderer,
            p.xs.data(),
            p.ys.data(),
            p.xs.size(),
            240, 160, 0, SDL_ALPHA_OPAQUE
        );
    }
}

void drawPolyFilled(SDL_Renderer *renderer, const std::vector<Polygon> &polys) {
    for (std::size_t i = 0; i < polys.size(); i++) {
        const Polygon& p = polys[i];
        filledPolygonColor(
            renderer,
            p.xs.data(),
            p.ys.data(),
            p.xs.size(),
            p.color
        );
    }
}

void draw(SDL_Renderer *renderer, const Application &app)
{
    /* Remplissez cette fonction pour faire l'affichage du jeu */
    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);

    if (app.voronoi) {
        if (app.filled) {
            drawPolyFilled(renderer, app.polygons);
        } else {
            drawPoly(renderer, app.polygons);
        }
        drawPoints(renderer, app.points);
    } else {
        if (app.filled) {
            drawTrianglesFilled(renderer, app.triangles);
        } else {
            drawPoints(renderer, app.points);
            drawTriangles(renderer, app.triangles);
        }
    }
}

/*
   Détermine si un point se trouve dans un cercle définit par trois points
   Retourne, par les paramètres, le centre et le rayon
*/
bool CircumCircle(
    float pX, float pY,
    float x1, float y1, float x2, float y2, float x3, float y3,
    float *xc, float *yc, float *rsqr
) {
    float m1, m2, mx1, mx2, my1, my2;
    float dx, dy, drsqr;
    float fabsy1y2 = fabs(y1 - y2);
    float fabsy2y3 = fabs(y2 - y3);

    /* Check for coincident points */
    if (fabsy1y2 < EPSILON && fabsy2y3 < EPSILON)
        return (false);

    if (fabsy1y2 < EPSILON) {
        m2 = -(x3 - x2) / (y3 - y2);
        mx2 = (x2 + x3) / 2.0;
        my2 = (y2 + y3) / 2.0;
        *xc = (x2 + x1) / 2.0;
        *yc = m2 * (*xc - mx2) + my2;
    } else if (fabsy2y3 < EPSILON) {
        m1 = -(x2 - x1) / (y2 - y1);
        mx1 = (x1 + x2) / 2.0;
        my1 = (y1 + y2) / 2.0;
        *xc = (x3 + x2) / 2.0;
        *yc = m1 * (*xc - mx1) + my1;
    } else {
        m1 = -(x2 - x1) / (y2 - y1);
        m2 = -(x3 - x2) / (y3 - y2);
        mx1 = (x1 + x2) / 2.0;
        mx2 = (x2 + x3) / 2.0;
        my1 = (y1 + y2) / 2.0;
        my2 = (y2 + y3) / 2.0;
        *xc = (m1 * mx1 - m2 * mx2 + my2 - my1) / (m1 - m2);
        if (fabsy1y2 > fabsy2y3) {
            *yc = m1 * (*xc - mx1) + my1;
        } else {
            *yc = m2 * (*xc - mx2) + my2;
        }
    }

    dx = x2 - *xc;
    dy = y2 - *yc;
    *rsqr = dx * dx + dy * dy;

    dx = pX - *xc;
    dy = pY - *yc;
    drsqr = dx * dx + dy * dy;

    return ((drsqr - *rsqr) <= EPSILON ? true : false);
}

bool CircumCircle(Coords p, Triangle t, float *cx, float *cy, float *rs) {
    return CircumCircle(p.x, p.y, t.p1.x, t.p1.y, t.p2.x, t.p2.y, t.p3.x, t.p3.y, cx, cy, rs);
}

void construitDelaunay(Application &app) {
//    std::sort(app.points.begin(), app.points.end(), compareCoords);
    app.triangles.clear();

    app.triangles.push_back({{-1000, -1000}, {500, 3000}, {1500, -1000}});

    for (unsigned i = 0; i < app.points.size(); ++i) {
        std::vector<Segment> segments;

        for (auto it = app.triangles.begin(); it != app.triangles.end();) {
            float cx, cy, rs;

            if (CircumCircle(app.points[i], *it, &cx, &cy, &rs)) {
                segments.push_back({it->p1, it->p2});
                segments.push_back({it->p2, it->p3});
                segments.push_back({it->p1, it->p3});

                app.triangles.erase(it);
            } else {
                ++it;
            }
        }

        for (unsigned j = 0; j < segments.size();) {
            bool deletion = false;
            for (unsigned k = j+1; k < segments.size();) {
                if (segments[j] == segments[k]) {
                    deletion = true;
                    segments.erase(segments.begin() + k);
                } else {
                    ++k;
                }
            }

            if (deletion) {
                segments.erase(segments.begin() + j);
            } else {
                ++j;
            }
        }

        for (unsigned j = 0; j < segments.size(); ++j) {
            app.triangles.push_back({segments[j].p1, segments[j].p2, app.points[i]});
        }
    }

    for (unsigned i = 0; i < app.triangles.size(); ++i) {
        app.triangles[i].color = 255 + (dis(generator) << 8);
    }
}

void construitVoronoi(Application &app) {
    construitDelaunay(app);

    app.polygons.clear();
    for (Coords pt : app.points) {

        unsigned i = 0;
        for (; i < app.triangles.size() && !app.triangles[i].hasPoint(pt); ++i) {}

        Triangle &t = app.triangles[i];
        Coords pt2 = t.getOther(pt);
        Coords pt3 = t.getOther(pt, pt2);
        float cx, cy, rs;
        CircumCircle(pt, t, &cx, &cy, &rs);
        std::vector<Sint16> polyXs{static_cast<Sint16>(cx)};
        std::vector<Sint16> polyYs{static_cast<Sint16>(cy)};

        Coords pt4 = pt2;

        while (pt2 != pt3) {
            bool searching = true;
            for (unsigned j = i+1; searching && j < app.triangles.size(); ++j) {
                Triangle &t2 = app.triangles[j];
                if (t2.hasPoints(pt, pt3) && !t2.hasPoint(pt4)) {
                    pt4 = pt3;
                    pt3 = t2.getOther(pt, pt4);
                    CircumCircle(pt, t2, &cx, &cy, &rs);
                    polyXs.push_back(static_cast<Sint16>(cx));
                    polyYs.push_back(static_cast<Sint16>(cy));
                    searching = false;
                }
            }
        }
        app.polygons.push_back(Polygon{polyXs, polyYs, 255 + (dis(generator) << 8)});
    }

}

bool handleEvent(Application &app) {
    /* Remplissez cette fonction pour gérer les inputs utilisateurs */
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.scancode == SDL_SCANCODE_ESCAPE))
            return false;
        else if (e.type == SDL_WINDOWEVENT_RESIZED) {
            app.width = e.window.data1;
            app.height = e.window.data1;
        } else if (e.type == SDL_KEYDOWN && e.key.keysym.scancode == SDL_SCANCODE_F) {
            app.filled = !app.filled;
        } else if (e.type == SDL_KEYDOWN && e.key.keysym.scancode == SDL_SCANCODE_V) {
            app.voronoi = !app.voronoi;
        } else if (e.type == SDL_MOUSEBUTTONUP) {
            if (e.button.button == SDL_BUTTON_RIGHT) {
                app.focus.x = e.button.x;
                app.focus.y = e.button.y;
                app.points.clear();
            } else if (e.button.button == SDL_BUTTON_LEFT) {
                app.focus.y = 0;
                app.points.push_back(Coords{e.button.x, e.button.y});
                construitVoronoi(app);
            }
        }
    }
    return true;
}

int main() {
    SDL_Window *gWindow;
    SDL_Renderer *renderer;
    Application app{720, 720, Coords{0, 0}};
    bool is_running = true;

    // Creation de la fenetre
    gWindow = init("Awesome Voronoi", 720, 720);

    if (!gWindow) {
        SDL_Log("Failed to initialize!\n");
        exit(1);
    }

    renderer = SDL_CreateRenderer(gWindow, -1, 0); // SDL_RENDERER_PRESENTVSYNC

    /*  GAME LOOP  */
    while (true) {
        // INPUTS
        is_running = handleEvent(app);
        if (!is_running)
            break;

        // EFFACAGE FRAME
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // DESSIN
        draw(renderer, app);

        // VALIDATION FRAME
        SDL_RenderPresent(renderer);

        // PAUSE en ms
        SDL_Delay(1000 / 30);
    }

    // Free resources and close SDL
    close(gWindow, renderer);

    return 0;
}
