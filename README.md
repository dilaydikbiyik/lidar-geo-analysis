# 📍 Lidar Geo-Analysis

Bu proje, 2 Boyutlu LiDAR taramalarından elde edilen nokta bulutu (point cloud) verilerini okuyarak analiz eden, RANSAC algoritması ile bu noktalardan geometrik yapılar (doğru parçaları, duvarlar, koridorlar vb.) çıkaran ve sonuçları SVG formatında görselleştiren modüler bir C++ uygulamasıdır. Proje, MVC (Model-View-Controller) mimarisi prensiplerine uygun olarak tasarlanmıştır.

## 🚀 Özellikler

- **TOML Veri Formatı Desteği:** Ham LiDAR tarama verilerini yapılandırılmış `.toml` dosyalarından okur ve ayrıştırır.
- **Polar - Kartezyen Dönüşümü:** Sensörden gelen uzaklık ve açı (polar) koordinatlarındaki veriyi işlenebilir Kartezyen ($x, y$) koordinat düzlemine çevirir.
- **RANSAC Algoritması (Çizgi Çıkarımı):** Gürültülü nokta bulutu verileri içinde iteratif istatistiksel yöntemlerle en uygun doğru parçası denklemlerini bularak ortamdaki fiziksel engelleri (duvar, sınır vb.) tespit eder.
- **Geometrik Kesişim Analizi:** Çıkarılan doğru parçalarının kesişim noktalarını ve aralarındaki açıları bularak haritadaki köşe (corner) noktalarını hesaplar.
- **Dinamik SVG Görselleştirme:** Elde edilen noktalar, tespit edilen çizgiler ve kesişim noktaları ölçeklenebilir vektörel bir SVG dosyası olarak renkli biçimde dışa aktarılır.
- **Uzaktan Veri İndirme Desteği:** Yalnızca yerel dosyaları değil, bir URL üzerinden sunulan veri dosyalarını da çekerek işleyebilir. 

## 🗂️ Proje Yapısı

Proje dosyaları mantıksal bir MVC düzeninde klasörlere ayrılmıştır:

```text
lidar-geo-analysis/
├── CMakeLists.txt        # Proje genel CMake derleme yapılandırması
├── README.md             # Proje dokümantasyonu
├── data/                 # Örnek LiDAR veri dosyaları (.toml uzantılı)
├── src/                  # Kaynak kod dizini
│   ├── main.cpp          # Ana yürütülebilir dosya (Uygulamanın giriş noktası)
│   ├── controller/       # MVC - Kontrolcü katmanı (Veri akışı ve iş mantığı yönetimi)
│   ├── model/            # MVC - Veri ve Algoritma katmanı (Geometry, Ransac, Lidar, Toml Parser vb.)
│   ├── utils/            # Yardımcı araçlar (CLI parametre ayrıştırıcı, Ağ işlemleri)
│   └── view/             # MVC - Sunum katmanı (Console View, SVG Writer vb.)
└── tests/                # Birim (Unit) testlerinin bulunduğu klasör ve kendi CMakeLists'i
```

## 🛠️ Kurulum ve Derleme (Build) Gereksinimleri

- **C++ Sürümü:** C++17 veya daha yenisi destekleyen bir derleyici (GCC, Clang, MSVC)
- **Derleme Sistemi:** CMake (Tavsiye edilen minimum sürüm 3.10)

### ⚙️ Derleme Adımları (Linux / MacOS / Windows)

Terminal veya komut istemcisinde proje kök dizininde sırasıyla şu komutları çalıştırın:

```bash
# 1. Derleme dosyaları için bir klasör oluşturun ve içine girin:
mkdir build
cd build

# 2. CMake konfigürasyonunu başlatarak Make/Ninja/VS dosyalarını üretin:
cmake ..

# 3. Projeyi derleyin:
cmake --build .
# (Linux/MacOS ortamlarında alternatif olarak 'make' komutunu da kullanabilirsiniz)
```

Derleme işlemi bittikten sonra `build` klasörü içinde platformunuza göre `proje_calistir` (veya `proje_calistir.exe`, proje ismine göre değişebilir) isimli çalıştırılabilir dosya elde edilecektir. Eş zamanlı olarak CTest testleri de yapılandırılır.

## 💻 Kullanım (CLI Seçenekleri)

Oluşturulan CLI uygulaması çeşitli parametrelerle LiDAR taramalarının nasıl işleneceğine müdahale etmenizi sağlar.

**Temel Kullanım:**
```bash
# Yerel bir TOML dosyasını işleyip haritayı SVG olarak dışa aktarmak
./proje_calistir -i ../data/lidar1.toml -o cikti_harita.svg
```

**Web Üzerinden Veri Okuma:**
```bash
# URL üzerinden TOML dosyası alıp işlemek
./proje_calistir -i https://example.com/lidar_test.toml -o web_harita.svg
```

**Gelişmiş RANSAC Ayarları ile Kullanım:**
RANSAC algoritmasının tolerans (epsilon), iterasyon sayısı veya en az nokta (min inliers) gibi kritik parametrelerine ince ayar yaparak model kalitesini artırabilirsiniz:
```bash
./proje_calistir -i ../data/lidar_test.toml --minInliers 15 --epsilon 0.1 --maxIters 1000 -o detayli_cikti.svg
```

## 🧪 Birim Testleri Çalıştırma

Projenin güvenilirliğini ve temel matematik fonksiyonlarını sınamak adına yazılmış CTest uyumlu birim testleri (Unit Tests) bulunmaktadır. Testleri çalıştırmak için `build` dizininde:

```bash
cd build
ctest --output-on-failure
# veya test yürütülebilir dosyalarını (ör: tests/test_geometry) doğrudan çalıştırabilirsiniz.
```

## 👥 Katkı Sağlama (Contributing)

Geliştirmelere (örn. RANSAC verimliliğinin artırılması, GUI arayüzü eklenmesi, farklı tarayıcı formatlarının desteklenmesi) dair Pull Request (PR) göndererek katkıda bulunabilirsiniz. Raporlamak istediğiniz hatalar için "Issues" bölümünü kullanabilirsiniz.