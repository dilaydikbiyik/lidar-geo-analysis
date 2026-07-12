# Lidar Geo-Analysis

A modular C++ application that reads 2D LiDAR point-cloud scans, extracts geometric structures (line segments, walls, corridors, etc.) from them using the RANSAC algorithm, and renders the results as SVG. The project follows an MVC (Model-View-Controller) architecture.

## Features

- **TOML data format support** — reads and parses raw LiDAR scan data from structured `.toml` files.
- **Polar-to-Cartesian conversion** — converts sensor range/angle (polar) readings into a processable Cartesian (x, y) coordinate plane.
- **RANSAC line extraction** — iteratively fits the best line-segment equations to noisy point-cloud data to detect physical obstacles (walls, boundaries, etc.).
- **Geometric intersection analysis** — computes intersection points and angles between extracted line segments to identify corner points on the map.
- **Dynamic SVG visualization** — exports the resulting points, detected lines, and intersection points as a scalable, color-coded SVG file.
- **Remote data fetching** — can process not only local files but also data served over a URL.

## Project Structure

The project is organized into a logical MVC layout:

```text
lidar-geo-analysis/
├── CMakeLists.txt        # Top-level CMake build configuration
├── README.md
├── data/                 # Sample LiDAR data files (.toml)
├── src/
│   ├── main.cpp          # Application entry point
│   ├── controller/       # MVC — data flow and business logic
│   ├── model/            # MVC — data & algorithms (Geometry, RANSAC, Lidar, TOML parser)
│   ├── utils/             # Helpers (CLI argument parsing, networking)
│   └── view/              # MVC — presentation layer (console view, SVG writer)
└── tests/                 # Unit tests and their own CMakeLists.txt
```

## Build Requirements

- **C++ standard:** C++17 or newer (GCC, Clang, or MSVC)
- **Build system:** CMake 3.10+ (recommended minimum)

### Build steps (Linux / macOS / Windows)

```bash
# 1. Create and enter a build directory
mkdir build
cd build

# 2. Configure the build (generates Make/Ninja/VS files)
cmake ..

# 3. Compile
cmake --build .
# On Linux/macOS you can alternatively run `make`
```

After a successful build, the `build` directory will contain an executable (name depends on the project's target, e.g. `lidar_geo_analysis` or `.exe` on Windows). CTest is configured alongside the build.

## Usage (CLI options)

**Basic usage — process a local TOML file and export the map as SVG:**
```bash
./lidar_geo_analysis -i ../data/lidar1.toml -o output_map.svg
```

**Reading data from a URL:**
```bash
./lidar_geo_analysis -i https://example.com/lidar_test.toml -o web_map.svg
```

**Advanced RANSAC tuning:**
Fine-tune RANSAC parameters such as tolerance (epsilon), iteration count, or minimum inliers to improve model quality:
```bash
./lidar_geo_analysis -i ../data/lidar_test.toml --minInliers 15 --epsilon 0.1 --maxIters 1000 -o detailed_output.svg
```

## Running Unit Tests

CTest-compatible unit tests cover the project's core geometry and math functions. From the `build` directory:

```bash
cd build
ctest --output-on-failure
# or run the test executables directly, e.g. tests/test_geometry
```

## Contributing

Contributions are welcome — for example, improving RANSAC performance, adding a GUI, or supporting additional scan formats. Please open a Pull Request, or use the Issues tab to report bugs.

## License

No license specified yet.
