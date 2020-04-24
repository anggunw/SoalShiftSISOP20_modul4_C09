# SoalShiftSISOP20_modul4_C09
* [Soal 1](#soal1)
      
* [Soal 2](#soal2)
      
* [Soal 3](#soal3)

* [Soal 4](#soal4)



## Soal 4 (Log System) <a name="soal4"></a>
```
void logFile(char* command, char* desc)
{
    char now[100];
    char level[30];
    time_t rawtime;
    struct tm *info;
    time(&rawtime);
    info = localtime(&rawtime);
    strftime(now, sizeof(now), "%y%m%d-%H:%M:%S::", info);
    if(strcmp(command, "RMDIR") == 0 || strcmp(command, "UNLINK") == 0){
        strcpy(level, "WARNING");
    }
    else{
        strcpy(level, "INFO");
    }
    char logLine[200];
    sprintf(logLine, "%s::%s%s::%s", level, now, command, desc);

    FILE* fp;
    fp = fopen("/home/farrelmt/fs.log", "a");
    fprintf(fp, "%s\n", logLine);
    fclose(fp);
}
```
Perintah soal 4 adalah membentuk file "fs.log"di direktori home yang menyimpan daftar perintah system call yang telah dijalankan. Oleh karena itu, ketika menjalankan fungsi xmp_mkdir dll, fungsi logFile dipanggil dengan parameter command dan desc. Parameter command adalah nama perintah yang dijalankan dan parameter desc adalah pathnya.

Di fungsi logFile, parameter command akan di compare dengan string "RMDIR" atau "UNLINK" dan jika memenuhi, maka variabel level akan menjadi "WARNING". Selain itu, variabel level akan menjadi "INFO". Informasi mengenai waktu system call dijalankan diambil dari localtime

Kemudian, fungsi fopen dijalankan untuk melakukan pencatatan log ke file fs.log sesuai dengan informasi yang telah didapatkan
