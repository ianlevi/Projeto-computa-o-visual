#include "common.h"
#include "image_ops.h"
#include "histogram.h"
#include "ui.h"
#include <stdio.h>
#include <string.h>

static void center_window(SDL_Window* w){
    SDL_SetWindowPosition(w, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

int main(int argc, char** argv){
    if(argc < 2){
        printf("Uso: %s caminho_da_imagem\n", argv[0]);
        return 1;
    }
    const char* imgpath = argv[1];

    if(SDL_Init(SDL_INIT_VIDEO) != 0){
        printf("SDL_Init erro: %s\n", SDL_GetError());
        return 1;
    }
    bool have_ttf = false;
    if(TTF_WasInit() == 0){ if(TTF_Init() == 0) have_ttf = true; } else have_ttf = true;

    SDL_Surface* src = load_image_rgba32(imgpath);
    if(!src){ printf("Falha ao carregar imagem: %s\n", imgpath); if(have_ttf) TTF_Quit(); SDL_Quit(); return 1; }

    SDL_Surface* gray = NULL;
    if(is_grayscale_surface(src)){
        gray = SDL_ConvertSurfaceFormat(src, SDL_PIXELFORMAT_RGBA32);
    } else {
        gray = convert_to_grayscale(src);
    }
    if(!gray){ printf("Falha ao obter imagem em cinza\n"); SDL_DestroySurface(src); if(have_ttf) TTF_Quit(); SDL_Quit(); return 1; }

    uint32_t hist[256]; compute_histogram(gray, hist);
    const int total_pixels = gray->w * gray->h;
    double mean = compute_mean(hist, total_pixels);
    double stdv = compute_stddev(hist, total_pixels, mean);
    const char* bright_class = (mean < 85.0) ? "ESCURA" : (mean < 170.0) ? "MEDIA" : "CLARA";
    const char* contrast_class = (stdv < 35.0) ? "BAIXO" : (stdv < 70.0) ? "MEDIO" : "ALTO";

    SDL_Window *win_img=NULL,*win_panel=NULL; SDL_Renderer *ren_img=NULL,*ren_panel=NULL;
    int imgw=gray->w, imgh=gray->h; int winw=(imgw>1200?1200:imgw), winh=(imgh>800?800:imgh);
    if(!create_window_renderer("Imagem - Principal", winw, winh, &win_img,&ren_img)){ SDL_DestroySurface(src); SDL_DestroySurface(gray); if(have_ttf) TTF_Quit(); SDL_Quit(); return 1; }

    center_window(win_img);

    int panel_w=360, panel_h=winh;
    if(!create_window_renderer("Painel - Histograma", panel_w, panel_h, &win_panel,&ren_panel)){
        SDL_DestroyRenderer(ren_img); SDL_DestroyWindow(win_img);
        SDL_DestroySurface(src); SDL_DestroySurface(gray); if(have_ttf) TTF_Quit(); SDL_Quit(); return 1;
    }
    SDL_SetWindowAlwaysOnTop(win_panel, true);
    int mx,my; SDL_GetWindowPosition(win_img,&mx,&my);
    SDL_SetWindowPosition(win_panel, mx + winw + 10, my);

    TTF_Font* font=NULL;
    if(have_ttf){
        const char* fpath = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
        font = TTF_OpenFont(fpath, 16);
        if(!font) SDL_Log("TTF_OpenFont falhou: %s", TTF_GetError());
    }

    SDL_Rect btn={20,10,panel_w-40,40}; ButtonState btn_state=BTN_IDLE; bool equalized=false;
    uint8_t lut[256]; make_equalization_lut(hist,total_pixels,lut); SDL_Surface* work = SDL_ConvertSurfaceFormat(gray, SDL_PIXELFORMAT_RGBA32);

    bool quit=false;
    while(!quit){
        SDL_Event e;
        while(SDL_PollEvent(&e)){
            if(e.type==SDL_EVENT_QUIT) quit=true;
            else if(e.type==SDL_EVENT_KEY_DOWN){
                if(e.key.key==SDLK_ESCAPE) quit=true;
                if(e.key.key==SDLK_s){ bool ok = save_surface_png_or_bmp("output_image.png",work); SDL_Log("Salvar: %s", ok? "OK":"FALHA"); }
                if(e.key.key==SDLK_e){
                    if(!equalized){ apply_lut_inplace(work,lut); compute_histogram(work,hist); equalized=true; }
                    else { SDL_DestroySurface(work); work=SDL_ConvertSurfaceFormat(gray, SDL_PIXELFORMAT_RGBA32); compute_histogram(work,hist); equalized=false; }
                    mean = compute_mean(hist, total_pixels);
                    stdv = compute_stddev(hist, total_pixels, mean);
                    bright_class = (mean < 85.0) ? "ESCURA" : (mean < 170.0) ? "MEDIA" : "CLARA";
                    contrast_class = (stdv < 35.0) ? "BAIXO" : (stdv < 70.0) ? "MEDIO" : "ALTO";
                }
            } else if(e.type==SDL_EVENT_WINDOW_MOVED || e.type==SDL_EVENT_WINDOW_SIZE_CHANGED){
                if(e.window.windowID == SDL_GetWindowID(win_img)){
                    SDL_GetWindowPosition(win_img,&mx,&my);
                    SDL_GetWindowSize(win_img,&winw,&winh);
                    SDL_SetWindowPosition(win_panel, mx + winw + 10, my);
                    panel_h = winh;
                }
            } else if(e.type==SDL_EVENT_MOUSE_MOTION || e.type==SDL_EVENT_MOUSE_BUTTON_DOWN || e.type==SDL_EVENT_MOUSE_BUTTON_UP){
                if(e.window.windowID == SDL_GetWindowID(win_panel)){
                    int px = (e.type==SDL_EVENT_MOUSE_MOTION)? e.motion.x : e.button.x;
                    int py = (e.type==SDL_EVENT_MOUSE_MOTION)? e.motion.y : e.button.y;
                    bool inside = (px>=btn.x && px<=btn.x+btn.w && py>=btn.y && py<=btn.y+btn.h);
                    if(e.type==SDL_EVENT_MOUSE_MOTION) btn_state = inside? BTN_HOVER: BTN_IDLE;
                    if(e.type==SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button==SDL_BUTTON_LEFT && inside) btn_state=BTN_PRESSED;
                    if(e.type==SDL_EVENT_MOUSE_BUTTON_UP && e.button.button==SDL_BUTTON_LEFT){
                        if(btn_state==BTN_PRESSED && inside){
                            if(!equalized){ apply_lut_inplace(work,lut); compute_histogram(work,hist); equalized=true; }
                            else { SDL_DestroySurface(work); work=SDL_ConvertSurfaceFormat(gray, SDL_PIXELFORMAT_RGBA32); compute_histogram(work,hist); equalized=false; }
                            mean = compute_mean(hist, total_pixels);
                            stdv = compute_stddev(hist, total_pixels, mean);
                            bright_class = (mean < 85.0) ? "ESCURA" : (mean < 170.0) ? "MEDIA" : "CLARA";
                            contrast_class = (stdv < 35.0) ? "BAIXO" : (stdv < 70.0) ? "MEDIO" : "ALTO";
                        }
                        btn_state=BTN_IDLE;
                    }
                }
            }
        }

        int vw,vh; SDL_GetRenderOutputSize(ren_img,&vw,&vh);
        SDL_SetRenderDrawColor(ren_img,20,20,20,255); SDL_RenderClear(ren_img);
        draw_image_fit(ren_img, work, vw, vh);
        SDL_RenderPresent(ren_img);

        SDL_SetRenderDrawColor(ren_panel, 40,40,40,255); SDL_RenderClear(ren_panel);
        const char* label = equalized? "Original (Desfazer)" : "Equalizar (E)";
        draw_button(ren_panel, btn, label, btn_state, font);

        char info[256];
        snprintf(info, sizeof(info),
                 "Media: %.1f (%s)\nDesvio: %.1f (%s)\nS para salvar",
                 mean, bright_class, stdv, contrast_class);
        if(font) draw_multiline_text(ren_panel, font, info, 20, btn.y + btn.h + 12, 6);

        draw_histogram_ui(ren_panel, hist, 10, panel_h/3, 360 - 20, panel_h*2/3 - 20);
        SDL_RenderPresent(ren_panel);

        SDL_Delay(16);
    }

    SDL_DestroySurface(src);
    SDL_DestroySurface(gray);
    SDL_DestroySurface(work);
    if(font) TTF_CloseFont(font);
    if(have_ttf) TTF_Quit();
    SDL_DestroyRenderer(ren_panel); SDL_DestroyWindow(win_panel);
    SDL_DestroyRenderer(ren_img); SDL_DestroyWindow(win_img);
    SDL_Quit();
    return 0;
}
