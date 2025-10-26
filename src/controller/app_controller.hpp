#pragma once
#include "utils/cli.hpp"

class AppController {
public:
    // Constructor: main.cpp'den CLI parametrelerini alır
    AppController(const CliParams& params);

    // Ana uygulama akışı
    void run();

private:
    CliParams m_params; // Parametreleri saklamak için
};