#include "utils/cli.hpp"
#include "controller/app_controller.hpp"
#include <iostream>
#include <stdexcept> // std::exception için

int main(int argc, char* argv[]) {
    // 1. Yazdığınız CLI parser'ını çağırın
    std::optional<CliParams> params = parse_cli(argc, argv);

    // Eğer kullanıcı --help dediyse veya parametre hatası yaptıysa,
    // parse_cli zaten std::nullopt döndürür ve yardım mesajını basar.
    if (!params.has_value()) {
        return 1; // Hata ile çık
    }

    // 2. Controller'ı oluşturun ve işi ona devredin
    AppController controller(params.value());

    try {
        controller.run(); // Tüm uygulama akışını başlat
    } catch (const std::exception& e) {
        std::cerr << "Uygulama hatasi: " << e.what() << std::endl;
        return 1;
    }

    return 0; // Başarıyla tamamlandı
}