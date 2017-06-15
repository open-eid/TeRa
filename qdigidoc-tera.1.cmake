.\" Manpage for qdigidoc-tera
.TH qdigidoc-tera 1 "${BUILD_DATE}" "${VERSION}" "qdigidoc-tera man page"
.SH NAME
qdigidoc-tera \- Command line utility for re-timestamping Estonian DDOC digitally signed documents
.SH SYNOPSIS
qdigidoc-tera [OPTIONS]
.SH OPTIONS
  -v, --version                    Displays version information.
  -h, --help                       Displays this help.
  --file_in <file_in>              file to be time-stamped
  --dir_in <dir_in>                input directory (*.ddoc recursiveness can be
                                   determined with option 'R')
  -R                               if set then input directories are searched
                                   recursively
  --ts_url <ts_url>                time server url (default
                                   https://puhver.ria.ee/tsa)
  --ext_out <ext_out>              extension for output file (default 'asics')
  --file_out <file_out>            output file, can only be used with --file_in
                                   (default <file_in>.<ext_out>)
  --excl_dir <excl_dir>            directories to exclude from file search
  --no_ini_excl_dirs               if set exclude directories from config file
                                   are not taken into account
  --log_level <log_level>          console log level, default 'info' (possible
                                   values: none, error, warn, info, debug,
                                   trace)
  --logfile_level <logfile_level>  logfile log level, default 'info' (possible
                                   values: none, error, warn, info, debug,
                                   trace)
  --logfile_dir <logfile_dir>      logfile directory (default is current
                                   directory)
.SH SEE ALSO
qdigidoc-tera-gui(1), digidoc-tool(1), qdigidocclient(1), qdigidoccrypto(1)
