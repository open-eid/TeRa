#include <QString>

class Bdoc10Handler {
public:
    static bool isBdoc10Container(QString const& filePath);
    static bool isBdoc10Manifest(QByteArray const& manifest);
};