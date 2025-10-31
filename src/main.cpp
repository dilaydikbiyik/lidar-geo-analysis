#include "utils/cli.hpp"
#include "controller/app_controller.hpp"
#include <iostream>
#include <stdexcept>

int main(int argc, char* argv[]) {
    std::optional<CliParams> params = parse_cli(argc, argv);

    if (!params.has_value()) {
        return 1;
    }

    AppController controller(params.value());

    try {
        controller.run(); // Tüm uygulama akışını başlat
    } catch (const std::exception& e) {
        std::cerr << "Uygulama hatasi: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}