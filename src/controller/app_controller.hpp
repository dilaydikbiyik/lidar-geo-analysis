#pragma once
#include "utils/cli.hpp"

class AppController {
public:
    // CLI parametreleri
    AppController(const CliParams& params);

    // Ana uygulama akışı
    void run();

private:
    CliParams m_params;
};