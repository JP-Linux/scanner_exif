#include <exiv2/exiv2.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>
#include <memory>
#include <ctime>
#include <cmath>
#include <sys/stat.h>  // Para struct stat

// Estrutura para configuração do programa
struct Config {
    bool showExif = true;
    bool showIptc = true;
    bool showXmp = true;
    bool showComments = true;
    bool showBasic = true;
    bool showGps = true;
    bool verbose = false;
    bool jsonOutput = false;
    std::string filename;
};

// Tipo compatível com diferentes versões do Exiv2
#if EXIV2_TEST_VERSION(0,27,0)
using ImagePtr = Exiv2::Image::UniquePtr;
#else
using ImagePtr = Exiv2::Image::AutoPtr;
#endif

// Conversor de data EXIF para formato legível
std::string formatExifDate(const std::string& exifDate) {
    if (exifDate.empty() || exifDate.length() < 19) return exifDate;

    return exifDate.substr(0, 4) + "-" +
    exifDate.substr(5, 2) + "-" +
    exifDate.substr(8, 2) + " " +
    exifDate.substr(11, 8);
}

// Formata tamanhos de arquivo de forma legível
std::string formatFileSize(uint64_t size) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit = 0;
    double fsize = static_cast<double>(size);

    while (fsize > 1024 && unit < 4) {
        fsize /= 1024;
        unit++;
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(unit > 0 ? 1 : 0) << fsize << " " << units[unit];
    return oss.str();
}

// Converte coordenadas GPS para formato decimal
std::string formatGpsCoordinate(const Exiv2::ExifData& exifData) {
    try {
        // Verifica se existem tags GPS
        auto latRef = exifData.findKey(Exiv2::ExifKey("Exif.GPSInfo.GPSLatitudeRef"));
        auto lonRef = exifData.findKey(Exiv2::ExifKey("Exif.GPSInfo.GPSLongitudeRef"));

        if (latRef == exifData.end() || lonRef == exifData.end()) {
            return "Não disponível";
        }

        // Obtém valores de latitude
        auto latVal = exifData.findKey(Exiv2::ExifKey("Exif.GPSInfo.GPSLatitude"));
        if (latVal == exifData.end() || latVal->count() != 3) {
            return "Dados incompletos";
        }

        // Obtém valores de longitude
        auto lonVal = exifData.findKey(Exiv2::ExifKey("Exif.GPSInfo.GPSLongitude"));
        if (lonVal == exifData.end() || lonVal->count() != 3) {
            return "Dados incompletos";
        }

        // Converte para graus decimais
        double lat = latVal->toFloat(0) + (latVal->toFloat(1)/60) + (latVal->toFloat(2)/3600);
        double lon = lonVal->toFloat(0) + (lonVal->toFloat(1)/60) + (lonVal->toFloat(2)/3600);

        // Ajusta para hemisférios
        if (latRef->toString() == "S") lat = -lat;
        if (lonRef->toString() == "W") lon = -lon;

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(6)
        << lat << ", " << lon;

        return oss.str();

    } catch (...) {
        return "Erro na conversão";
    }
}

// Obtém informações básicas do arquivo
void getFileInfo(const std::string& filename, std::ostream& os) {
    struct stat fileInfo;
    if (stat(filename.c_str(), &fileInfo) != 0) {
        os << "  Tamanho do arquivo: Indisponível\n";
        return;
    }

    // Data de modificação
    char modTime[20];
    strftime(modTime, sizeof(modTime), "%Y-%m-%d %H:%M:%S", localtime(&fileInfo.st_mtime));

    os << "  Tamanho do arquivo: " << formatFileSize(fileInfo.st_size) << "\n";
    os << "  Última modificação: " << modTime << "\n";
}

// Analisa os argumentos da linha de comando
Config parseArguments(int argc, char* argv[]) {
    Config config;

    if (argc < 2) {
        throw std::runtime_error("Argumentos insuficientes");
    }

    // O último argumento é sempre o nome do arquivo
    config.filename = argv[argc - 1];

    // Verifica as flags
    for (int i = 1; i < argc - 1; ++i) {
        std::string arg = argv[i];
        std::transform(arg.begin(), arg.end(), arg.begin(),
                       [](unsigned char c){ return std::tolower(c); });

        if (arg == "--exif" || arg == "-e") {
            config.showIptc = false;
            config.showXmp = false;
            config.showComments = false;
            config.showBasic = false;
            config.showGps = false;
        } else if (arg == "--iptc" || arg == "-i") {
            config.showExif = false;
            config.showXmp = false;
            config.showComments = false;
            config.showBasic = false;
            config.showGps = false;
        } else if (arg == "--xmp" || arg == "-x") {
            config.showExif = false;
            config.showIptc = false;
            config.showComments = false;
            config.showBasic = false;
            config.showGps = false;
        } else if (arg == "--comments" || arg == "-c") {
            config.showExif = false;
            config.showIptc = false;
            config.showXmp = false;
            config.showBasic = false;
            config.showGps = false;
        } else if (arg == "--gps" || arg == "-g") {
            config.showExif = false;
            config.showIptc = false;
            config.showXmp = false;
            config.showComments = false;
            config.showBasic = false;
        } else if (arg == "--basic" || arg == "-b") {
            config.showExif = false;
            config.showIptc = false;
            config.showXmp = false;
            config.showComments = false;
            config.showGps = false;
        } else if (arg == "--all" || arg == "-a") {
            // Ativa tudo (já é padrão)
        } else if (arg == "--verbose" || arg == "-v") {
            config.verbose = true;
        } else if (arg == "--json" || arg == "-j") {
            config.jsonOutput = true;
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Uso: " << argv[0] << " [OPÇÕES] arquivo\n"
            << "Opções:\n"
            << "  -e, --exif\t\tMostrar apenas EXIF\n"
            << "  -i, --iptc\t\tMostrar apenas IPTC\n"
            << "  -x, --xmp\t\tMostrar apenas XMP\n"
            << "  -c, --comments\tMostrar apenas comentários\n"
            << "  -g, --gps\t\tMostrar apenas coordenadas GPS\n"
            << "  -b, --basic\t\tMostrar apenas informações básicas\n"
            << "  -a, --all\t\tMostrar todos (padrão)\n"
            << "  -v, --verbose\t\tModo verboso\n"
            << "  -j, --json\t\tSaída em formato JSON\n"
            << "  -h, --help\t\tMostrar esta ajuda\n";
            exit(0);
        } else {
            throw std::runtime_error("Opção desconhecida: " + arg);
        }
    }

    return config;
}

// Exibe metadados em formato legível
void printMetadata(const Exiv2::ExifData& data, const std::string& title) {
    if (data.empty()) {
        std::cout << "  ✗ " << title << ": Nenhum dado encontrado\n";
        return;
    }

    std::cout << "  ✓ " << title << ":\n";
    for (const auto& md : data) {
        std::string value = md.toString();

        // Formata datas EXIF
        if (md.tagName() == "DateTime" || md.tagName() == "DateTimeOriginal") {
            value = formatExifDate(value);
        }

        std::cout << "    • " << std::setw(35) << std::left << md.tagName()
        << " : " << value << "\n";
    }
}

// Exibe informações básicas da imagem
void printBasicInfo(ImagePtr& image, const Config& config) {
    std::cout << "\n\033[1;34m[Informações da Imagem]\033[0m\n";
    std::cout << "  Arquivo: " << config.filename << "\n";
    std::cout << "  Formato: " << image->mimeType() << "\n";

    if (image->pixelWidth() > 0 && image->pixelHeight() > 0) {
        std::cout << "  Dimensões: " << image->pixelWidth()
        << " × " << image->pixelHeight() << " pixels\n";
        std::cout << "  Proporção: " << std::fixed << std::setprecision(2)
        << static_cast<float>(image->pixelWidth()) / image->pixelHeight() << ":1\n";
    }

    getFileInfo(config.filename, std::cout);
}

// Exibe coordenadas GPS
void printGpsInfo(const Exiv2::ExifData& exifData) {
    std::string coordinates = formatGpsCoordinate(exifData);

    std::cout << "\n\033[1;34m[Coordenadas GPS]\033[0m\n";
    std::cout << "  Localização: " << coordinates << "\n";

    // Tenta obter o nome da localização
    auto locRef = exifData.findKey(Exiv2::ExifKey("Exif.GPSInfo.GPSAreaInformation"));
    if (locRef != exifData.end()) {
        std::cout << "  Área: " << locRef->toString() << "\n";
    }

    // Tenta obter a altitude - método mais compatível
    auto altRef = exifData.findKey(Exiv2::ExifKey("Exif.GPSInfo.GPSAltitudeRef"));
    auto altVal = exifData.findKey(Exiv2::ExifKey("Exif.GPSInfo.GPSAltitude"));

    if (altVal != exifData.end() && altRef != exifData.end()) {
        try {
            double altitude = altVal->toFloat();
            // GPSAltitudeRef: 0 = acima do nível do mar, 1 = abaixo do nível do mar
            int ref = 0;
            try {
                ref = static_cast<int>(altRef->toInt64());
            } catch (...) {
                // Tentar interpretar como string
                std::string refStr = altRef->toString();
                if (refStr == "1" || refStr == "Below sea level") ref = 1;
            }

            if (ref == 1) altitude = -altitude;
            std::cout << "  Altitude: " << std::fixed << std::setprecision(2)
            << altitude << " metros\n";
        } catch (const Exiv2::Error& e) {
            std::cout << "  Altitude: Erro na leitura (" << e.what() << ")\n";
        }
    }
}

// Exibe metadados completos
void printAllMetadata(ImagePtr& image, const Config& config) {
    printBasicInfo(image, config);

    // Exibe coordenadas GPS se disponíveis
    if (config.showGps && !image->exifData().empty()) {
        try {
            printGpsInfo(image->exifData());
        } catch (...) {
            std::cout << "\n\033[1;33m[Aviso] Erro ao processar dados GPS\033[0m\n";
        }
    }

    std::cout << "\n\033[1;34m[Metadados]\033[0m\n";
    if (config.showExif) printMetadata(image->exifData(), "EXIF");
    if (config.showIptc) {
        if (!image->iptcData().empty()) {
            std::cout << "  ✓ IPTC:\n";
            for (const auto& md : image->iptcData()) {
                std::cout << "    • " << std::setw(35) << std::left << md.tagName()
                << " : " << md.toString() << "\n";
            }
        } else if (config.showIptc) {
            std::cout << "  ✗ IPTC: Nenhum dado encontrado\n";
        }
    }
    if (config.showXmp) {
        if (!image->xmpData().empty()) {
            std::cout << "  ✓ XMP:\n";
            for (const auto& md : image->xmpData()) {
                std::cout << "    • " << std::setw(35) << std::left << md.tagName()
                << " : " << md.toString() << "\n";
            }
        } else if (config.showXmp) {
            std::cout << "  ✗ XMP: Nenhum dado encontrado\n";
        }
    }
    if (config.showComments && !image->comment().empty()) {
        std::cout << "  ✓ Comentários:\n    " << image->comment() << "\n";
    }
}

// Versão simplificada para JSON
void printJsonOutput(ImagePtr& image, const Config& config) {
    std::cout << "{\n";
    std::cout << "  \"file\": \"" << config.filename << "\",\n";
    std::cout << "  \"mime_type\": \"" << image->mimeType() << "\",\n";

    if (image->pixelWidth() > 0 && image->pixelHeight() > 0) {
        std::cout << "  \"width\": " << image->pixelWidth() << ",\n";
        std::cout << "  \"height\": " << image->pixelHeight() << ",\n";
    }

    // EXIF
    if (config.showExif && !image->exifData().empty()) {
        std::cout << "  \"exif\": {\n";
        bool first = true;
        for (const auto& md : image->exifData()) {
            if (!first) std::cout << ",\n";
            std::cout << "    \"" << md.tagName() << "\": \"" << md.toString() << "\"";
            first = false;
        }
        std::cout << "\n  },\n";
    }

    // GPS
    if (config.showGps && !image->exifData().empty()) {
        std::string gps = formatGpsCoordinate(image->exifData());
        if (gps != "Não disponível" && gps != "Dados incompletos") {
            std::cout << "  \"gps\": \"" << gps << "\",\n";
        }
    }

    // Comentários
    if (config.showComments && !image->comment().empty()) {
        std::cout << "  \"comment\": \"" << image->comment() << "\",\n";
    }

    // Remover última vírgula se necessário
    std::cout << "  \"status\": \"ok\"\n";
    std::cout << "}\n";
}

int main(int argc, char* argv[]) {
    try {
        Config config = parseArguments(argc, argv);

        if (config.verbose) {
            std::cout << "Processando: " << config.filename << "\n";
            std::cout << "Biblioteca Exiv2 v" << Exiv2::version() << "\n";
        }

        // Abre a imagem
        ImagePtr image = Exiv2::ImageFactory::open(config.filename);
        if (!image.get()) {
            throw std::runtime_error("Formato não suportado ou arquivo inválido");
        }

        image->readMetadata();

        // Verifica se é um arquivo suportado
        if (image->mimeType().empty()) {
            throw std::runtime_error("Tipo de arquivo não reconhecido");
        }

        // Saída em JSON
        if (config.jsonOutput) {
            printJsonOutput(image, config);
            return 0;
        }

        // Cabeçalho informativo
        std::cout << "\n\033[1;36m" << std::string(60, '=') << "\033[0m";
        std::cout << "\n\033[1;36m  ANALISADOR DE METADADOS - EXIV2 v"
        << Exiv2::version() << "\033[0m";
        std::cout << "\n\033[1;36m" << std::string(60, '=') << "\033[0m\n";

        // Exibe informações conforme configurado
        bool specificRequest = config.showExif || config.showIptc || config.showXmp ||
        config.showComments || config.showGps;

        if (config.showBasic) {
            printAllMetadata(image, config);
        } else if (specificRequest) {
            if (config.showExif) printMetadata(image->exifData(), "EXIF");
            if (config.showIptc) {
                if (!image->iptcData().empty()) {
                    std::cout << "  ✓ IPTC:\n";
                    for (const auto& md : image->iptcData()) {
                        std::cout << "    • " << std::setw(35) << std::left << md.tagName()
                        << " : " << md.toString() << "\n";
                    }
                } else {
                    std::cout << "  ✗ IPTC: Nenhum dado encontrado\n";
                }
            }
            if (config.showXmp) {
                if (!image->xmpData().empty()) {
                    std::cout << "  ✓ XMP:\n";
                    for (const auto& md : image->xmpData()) {
                        std::cout << "    • " << std::setw(35) << std::left << md.tagName()
                        << " : " << md.toString() << "\n";
                    }
                } else {
                    std::cout << "  ✗ XMP: Nenhum dado encontrado\n";
                }
            }
            if (config.showComments) {
                if (!image->comment().empty()) {
                    std::cout << "  ✓ Comentários:\n    " << image->comment() << "\n";
                } else {
                    std::cout << "  ✗ Comentários: Nenhum comentário encontrado\n";
                }
            }
            if (config.showGps) {
                printGpsInfo(image->exifData());
            }
        } else {
            // Nenhuma opção específica selecionada, mostrar tudo
            printAllMetadata(image, config);
        }

        // Rodapé
        std::cout << "\n\033[1;36m" << std::string(60, '=') << "\033[0m\n";
        std::cout << "\033[1;32mAnálise concluída com sucesso!\033[0m\n";
        std::cout << "\033[1;36m" << std::string(60, '=') << "\033[0m\n";

    } catch (const Exiv2::Error& e) {
        std::cerr << "\n\033[1;31mERRO (Exiv2): " << e.what() << "\033[0m" << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "\n\033[1;31mERRO: " << e.what() << "\033[0m" << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "\n\033[1;31mERRO desconhecido\033[0m" << std::endl;
        return 1;
    }

    return 0;
}
