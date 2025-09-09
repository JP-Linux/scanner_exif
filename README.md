# Scanner Exif

[![GitHub Pages](https://img.shields.io/badge/GitHub%20Pages-Live-brightgreen)](https://jp-linux.github.io)
![C++](https://img.shields.io/badge/C++-17-blue.svg)
![Exiv2](https://img.shields.io/badge/Exiv2-0.27%2B-green.svg)
![Platform](https://img.shields.io/badge/Platform-Linux-lightgrey.svg)
![Arch Linux](https://img.shields.io/badge/Arch_Linux-Supported-brightgreen.svg)

Uma ferramenta de linha de comando em C++ para extrair e exibir metadados de imagens usando a biblioteca Exiv2. Suporta EXIF, IPTC, XMP, comentários, informações básicas de arquivo e coordenadas GPS com saída formatada em texto ou JSON.

## Recursos

- 📷 Extração de metadados EXIF, IPTC e XMP
- 📍 Conversão de coordenadas GPS para formato decimal
- 📊 Informações básicas da imagem (dimensões, formato, tamanho do arquivo)
- 💬 Exibição de comentários embutidos
- 📋 Saída formatada com destaque colorido (ANSI)
- 📦 Suporte a saída JSON para integração com outros sistemas
- ⚙️ Filtros seletivos para tipos específicos de metadados
- 🐧 Compatível com Arch Linux e outras distribuições Linux

## Pré-requisitos

- Compilador C++ com suporte a C++17
- Biblioteca Exiv2 (versão 0.27 ou superior)
- Sistema Linux (testado no Arch Linux)

### Instalação das dependências

#### Arch Linux
```bash
sudo pacman -S exiv2
```

#### Ubuntu/Debian
```bash
sudo apt install libexiv2-dev
```

## Compilação

```bash
# Clone o repositório
git clone https://github.com/JP-Linux/scanner_exif.git
cd scanner_exif

```

Para compilar manualmente:
```bash
g++ -std=c++17 -o scanner_exif scanner_exif.cpp $(pkg-config --cflags --libs exiv2)
```

## Uso

```
./scanner_exif [OPÇÕES] arquivo
```

### Opções disponíveis:
| Opção             | Descrição                                |
|-------------------|------------------------------------------|
| `-e, --exif`      | Mostrar apenas metadados EXIF            |
| `-i, --iptc`      | Mostrar apenas metadados IPTC            |
| `-x, --xmp`       | Mostrar apenas metadados XMP             |
| `-c, --comments`  | Mostrar apenas comentários               |
| `-g, --gps`       | Mostrar apenas coordenadas GPS           |
| `-b, --basic`     | Mostrar apenas informações básicas       |
| `-a, --all`       | Mostrar todos os metadados (padrão)      |
| `-v, --verbose`   | Modo verboso                             |
| `-j, --json`      | Saída em formato JSON                    |
| `-h, --help`      | Mostrar esta ajuda                       |

### Exemplos

Mostrar todos os metadados de uma imagem:
```bash
./scanner_exif foto.jpg
```

Mostrar apenas informações EXIF e GPS:
```bash
./scanner_exif --exif --gps foto.jpg
```

Gerar saída em JSON:
```bash
./scanner_exif --json foto.jpg > metadata.json
```

## Saída de Exemplo

```
============================================================
  ANALISADOR DE METADADOS - EXIV2 v0.27.5
============================================================

[Informações da Imagem]
  Arquivo: foto.jpg
  Formato: image/jpeg
  Dimensões: 4032 × 3024 pixels
  Proporção: 1.33:1
  Tamanho do arquivo: 4.2 MB
  Última modificação: 2023-10-15 14:30:22

[Coordenadas GPS]
  Localização: -23.550520, -46.633308

[Metadados]
  ✓ EXIF:
    • DateTimeOriginal              : 2023-10-15 12:05:23
    • ExposureTime                 : 1/100
    • FNumber                      : F2.8
    • ISOSpeedRatings              : 200
    • FocalLength                  : 50.0 mm
    • Make                         : Canon
    • Model                        : Canon EOS R5
    • LensModel                    : RF50mm F1.8 STM
    • GPSLatitudeRef               : S
    • GPSLongitudeRef              : W

  ✓ IPTC:
    • Keywords                     : natureza; paisagem
    • City                         : São Paulo
    • Country                      : Brasil
    • Copyright                    : © Jorge Paulo Santos

  ✓ XMP:
    • Rating                       : 4
    • CreatorTool                  : Adobe Photoshop 24.0
    • CreateDate                   : 2023-10-15T14:30:22

  ✓ Comentários:
    Foto tirada durante passeio no parque Ibirapuera

============================================================
Análise concluída com sucesso!
============================================================
```

## Saída JSON

```json
{
  "file": "foto.jpg",
  "mime_type": "image/jpeg",
  "width": 4032,
  "height": 3024,
  "exif": {
    "DateTimeOriginal": "2023:10:15 12:05:23",
    "ExposureTime": "1/100",
    "FNumber": "F2.8",
    "ISOSpeedRatings": "200",
    "FocalLength": "50.0 mm",
    "Make": "Canon",
    "Model": "Canon EOS R5",
    "LensModel": "RF50mm F1.8 STM",
    "GPSLatitudeRef": "S",
    "GPSLongitudeRef": "W"
  },
  "gps": "-23.550520, -46.633308",
  "comment": "Foto tirada durante passeio no parque Ibirapuera",
  "status": "ok"
}
```

## 👤 Autor

**Jorge Paulo Santos**  
[![GitHub](https://img.shields.io/badge/GitHub-100000?style=for-the-badge&logo=github&logoColor=white)](https://github.com/JP-Linux)
[![Email](https://img.shields.io/badge/Gmail-D14836?style=for-the-badge&logo=gmail&logoColor=white)](mailto:jorgepsan7@gmail.com)

## 💝 Suporte ao Projeto

Se este projeto foi útil para você, considere apoiar meu trabalho através do GitHub Sponsors:

[![Sponsor](https://img.shields.io/badge/Sponsor-JP_Linux-ea4aaa?style=for-the-badge&logo=githubsponsors)](https://github.com/sponsors/JP-Linux)


## Licença

Este projeto está licenciado sob a Licença MIT - veja o arquivo [LICENSE](LICENSE) para detalhes.

## Contato

**Jorge Paulo Santos**  
- GitHub: [JP-Linux](https://github.com/JP-Linux)  
- Email: jorgepsan7@gmail.com  
- Sistema: Arch Linux  
