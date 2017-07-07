#include "Bdoc10Handler.h"

#include <zip.h>

//Verifies that the file provided by parameter filePath is a BDOC10 container.
bool Bdoc10Handler::isBdoc10Container(QString const& filePath) {
    const char* BDOC_MIMETYPE_PATH = "mimetype";
    const char MIMETYPE_BDOC10_APPLICATION[] = "application/vnd.bdoc-1.0";

    bool bRes = false;
    zip *zipConatiner = zip_open(filePath.toUtf8().constData(), ZIP_RDONLY, NULL);
    if (NULL != zipConatiner) {
        zip_int64_t index = zip_name_locate(zipConatiner, BDOC_MIMETYPE_PATH, 0);
	    if (-1 != index) {
		    zip_file_t *zipfile = zip_fopen_index(zipConatiner, index, ZIP_FL_UNCHANGED);
		    if (NULL != zipConatiner) {
                QByteArray data(sizeof(MIMETYPE_BDOC10_APPLICATION), 0);
                zip_int64_t read = zip_fread(zipfile, data.data(), data.size());
                if (data.startsWith(MIMETYPE_BDOC10_APPLICATION)) {
                    bRes = true;
                }
		    }
	    }
	    zip_close(zipConatiner);
    }
    return bRes;
}
