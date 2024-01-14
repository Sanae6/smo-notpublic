set(URL "ftp://${FTP_USERNAME}:${FTP_PASSWORD}@${FTP_HOST}:${FTP_PORT}${FTP_PATH}")
execute_process(
        COMMAND curl -T ${FILE_TO_UPLOAD} ${URL}
)
message(STATUS "Uploaded ${FILE_TO_UPLOAD} to ${URL}")