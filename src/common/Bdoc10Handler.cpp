#include "Bdoc10Handler.h"

#include <zip.h>

//Verifies that the file provided by parameter filePath is a BDOC10 container.
bool Bdoc10Handler::isBdoc10Container(QString const& filePath) {
    const char* BDOC_MIMETYPE_PATH = "mimetype";
    const char* MIMETYPE_BDOC10_APPLICATION = "application/vnd.bdoc-1.0";

    bool bRes = false;
    int errorp = 0;
    zip *zipConatiner = zip_open(filePath.toUtf8().constData(), ZIP_RDONLY, &errorp);
    if (NULL != zipConatiner) {
        zip_int64_t index = zip_name_locate(zipConatiner, BDOC_MIMETYPE_PATH, 0);
	    if (-1 != index) {
		    zip_file_t *zipfile = zip_fopen_index(zipConatiner, index, ZIP_FL_UNCHANGED);
		    if (NULL != zipConatiner) {
			    const int FILE_CHUNK_SIZE = 256;
			    char buf[FILE_CHUNK_SIZE];
			    QByteArray data;
			    for (;;)
			    {
				    zip_int64_t read = zip_fread(zipfile, buf, FILE_CHUNK_SIZE);
                    if ((-1 == read) || (0 == read)) {
                        break;
				    }
                    else {
                        data.append(buf, read);
                    }
                }
                if (!data.isEmpty()) {
                    bRes = data.startsWith(MIMETYPE_BDOC10_APPLICATION);
			    }
		    }
	    }
	    zip_close(zipConatiner);
    }
    return bRes;
}
