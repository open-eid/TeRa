#include "Bdoc10Handler.h"

#include <QXmlStreamReader>
#include <zip.h>

//Verifies that the file provided by parameter filePath is a BDOC10 conatiner.
bool Bdoc10Handler::isBdoc10Container(QString const& filePath) {
    const QString BDOC_MANIFEST_PATH = "META-INF/manifest.xml";

    bool bRes = false;
    int errorp = 0;
    zip *zipConatiner = zip_open(filePath.toUtf8().constData(), ZIP_RDONLY, &errorp);
    if (NULL != zipConatiner) {
	    zip_int64_t index = zip_name_locate(zipConatiner, BDOC_MANIFEST_PATH.toUtf8().constData(), 0);
	    if (-1 != index) {
		    zip_file_t *zipfile = zip_fopen_index(zipConatiner, index, ZIP_FL_UNCHANGED);
		    if (NULL != zipConatiner) {
			    const int FILE_CHUNK_SIZE = 1024;
			    char buf[FILE_CHUNK_SIZE];
			    QByteArray xmlData;
			    for (;;)
			    {
				    zip_int64_t read = zip_fread(zipfile, buf, FILE_CHUNK_SIZE);
                    if ((-1 == read) || (0 == read)) {
                        break;
				    }
                    else {
                        xmlData.append(buf, read);
                    }
                }

			    if (!xmlData.isEmpty()) {
				    bRes = isBdoc10Manifest(xmlData);
			    }
		    }
	    }
	    zip_close(zipConatiner);
    }
    return bRes;
}

//Checks for specific to BDOC10 attributes in the manifest provided by parameter.
//<manifest:file-entry manifest:full-path="/" manifest:media-type="application/vnd.bdoc-1.0"/>
//and
//<manifest:file-entry manifest:full-path="META-INF/signature0.xml" manifest:media-type="signature/bdoc-1.0/TM"/>
bool Bdoc10Handler::isBdoc10Manifest(QByteArray const& manifest) {
    const QString MANIFEST_BDOC10_APPLICATION_TYPE = "application/vnd.bdoc-1.0";
    const QString MANIFEST_BDOC10_SIGNATURE_TYPE = "signature/bdoc-1.0/TM";

    bool bdoc10ApplicationType = false;
    bool bdoc10SignatureType = false;

    QXmlStreamReader xml(manifest);
    while (!xml.atEnd()) {
	    xml.readNext();
	    if (xml.isStartElement()) {
		    if ("file-entry" == xml.name()){
			    QXmlStreamAttributes attributes = xml.attributes();
			    QString attr = attributes.value("manifest:media-type").toString();
			    if (0 == MANIFEST_BDOC10_APPLICATION_TYPE.compare(attr, Qt::CaseInsensitive)) {
				    bdoc10ApplicationType = true;
			    }
			    else if (0 == MANIFEST_BDOC10_SIGNATURE_TYPE.compare(attr, Qt::CaseInsensitive)) {
				    bdoc10SignatureType = true;
			    }

			    if (bdoc10ApplicationType && bdoc10SignatureType)
				    break;
		    }
	    }
    }
    if (xml.hasError()) {
	    ;
    }
    return (bdoc10ApplicationType && bdoc10SignatureType);
}
