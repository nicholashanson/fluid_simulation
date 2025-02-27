#ifndef COLORS_HPP
#define COLORS_HPP

namespace lga {

    struct Color {

        float r;
        float g;
        float b;

    };

    inline const Color star_white{ 1.0f, 1.0f, 1.0f };
    inline const Color nebula_blue{ 0.0f, 0.3f, 1.0f };
    inline const Color cosmic_red{ 1.0f, 0.1f, 0.2f };
    inline const Color galactic_green{ 0.1f, 0.8f, 0.3f };
    inline const Color void_black = { 0.0f, 0.0f, 0.0f };
    inline const Color asteroid_gray = { 0.3f. 0.3f, 0.3f };
    inline const Color quasar_yellow = { 1.0f, 0.9f, 0.1f };
    inline const Color pulsar_magenta = { 0.9f, 0.1f, 1.0f };
    inline const Color plasma_cyan = { 0.1f, 0.9f, 1.0f };

    inline const std::unordered_map<std::pair<int,int>, Color> color_map = {
        {{-1,-1}, star_white },
        {{-1,0}, asteroid_gray },
        {{-1,1}, nebula_blue },
        {{0,-1}, cosmic_red },
        {{0,0}, void_black },
        {{0,1}, galactic_green },
        {{1,-1}, pulsar_magenta },
        {{1,0}, quasar_yellow },
        {{1,1}, plasma_cyan }
    };

}

#endif
