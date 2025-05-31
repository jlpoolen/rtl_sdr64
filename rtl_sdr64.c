// gcc -O2 -Wall -o rtl_sdr64 rtl_sdr64.c -lrtlsdr
// 2025-05-31 ChatGPT - $Header$
//
// 64-bit RTL-SDR IQ recorder with long-duration support and metadata logging
//

#include <rtl-sdr.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#define BUF_SIZE (256 * 1024)
#define DEFAULT_OUTPUT_DIR "/tmp/"

void usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s [options]\n"
        "  -s, --sample_rate   Sample rate (default: 2400000)\n"
        "  -f, --frequency     Frequency in Hz (default: 119100000)\n"
        "  -g, --gain          Gain in tenths of dB (default: 402)\n"
        "  -d, --device        RTL-SDR device index (default: 0)\n"
        "  -t, --time_seconds  Duration to capture in seconds (default: 3600)\n"
        "  -o, --output_dir    Output directory (default: /tmp/)\n"
        "  -n, --name          Output file prefix (default: ksle)\n",
        prog
    );
}

int main(int argc, char **argv) {
    uint32_t samp_rate = 2400000;
    uint32_t frequency = 119100000;
    int gain = 402;
    int device_index = 0;
    int seconds = 3600;
    char output_dir[256] = DEFAULT_OUTPUT_DIR;
    char name[64] = "ksle";  // default prefix

    static struct option long_options[] = {
        {"sample_rate",   required_argument, 0, 's'},
        {"frequency",     required_argument, 0, 'f'},
        {"gain",          required_argument, 0, 'g'},
        {"device",        required_argument, 0, 'd'},
        {"time_seconds",  required_argument, 0, 't'},
        {"output_dir",    required_argument, 0, 'o'},
        {"name",          required_argument, 0, 'n'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "s:f:g:d:t:o:n:", long_options, NULL)) != -1) {
        switch (opt) {
            case 's': samp_rate = (uint32_t)atoi(optarg); break;
            case 'f': frequency = (uint32_t)atof(optarg); break;
            case 'g': gain = atoi(optarg); break;
            case 'd': device_index = atoi(optarg); break;
            case 't': seconds = atoi(optarg); break;
            case 'o': strncpy(output_dir, optarg, sizeof(output_dir) - 1); break;
            case 'n': strncpy(name, optarg, sizeof(name) - 1); break;
            default: usage(argv[0]); return 1;
        }
    }

    // Ensure trailing slash on output directory
    size_t len = strlen(output_dir);
    if (output_dir[len - 1] != '/') {
        strcat(output_dir, "/");
    }

    // Format timestamp for filename
    char timestamp[32];
    time_t now = time(NULL);
    struct tm *tm_local = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%a_%H%M", tm_local);

    // Construct filename
    char filename[512];
    snprintf(filename, sizeof(filename), "%s%s_%s_%us_%dg.iq",
             output_dir, name, timestamp, samp_rate, gain);

    // Open device
    rtlsdr_dev_t *dev = NULL;
    if (rtlsdr_open(&dev, device_index) < 0) {
        fprintf(stderr, "Error: unable to open RTL-SDR device %u\n", device_index);
        return 1;
    }

    rtlsdr_set_sample_rate(dev, samp_rate);
    rtlsdr_set_center_freq(dev, frequency);
    rtlsdr_set_tuner_gain_mode(dev, 1);
    rtlsdr_set_tuner_gain(dev, gain);
    rtlsdr_reset_buffer(dev);

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    uint8_t *buf = malloc(BUF_SIZE);
    if (!buf) {
        perror("malloc");
        return 1;
    }

    uint64_t total_samples = (uint64_t)samp_rate * seconds;
    uint64_t total_bytes = total_samples * 2;
    uint64_t bytes_written = 0;

    // Write metadata sidecar
    char txtname[512];
    snprintf(txtname, sizeof(txtname), "%s.txt", filename);
    FILE *meta = fopen(txtname, "w");
    if (meta) {
        fprintf(meta, "provenance: %s built on %s", argv[0], __DATE__);
        fprintf(meta, "epoch_start: %ld\n", now);
        fprintf(meta, "time_start_local: %s", asctime(tm_local));
        fprintf(meta, "sample_rate: %u\n", samp_rate);
        fprintf(meta, "frequency: %u\n", frequency);
        fprintf(meta, "gain: %d\n", gain);
        fprintf(meta, "duration_sec: %d\n", seconds);
        fprintf(meta, "filename: %s\n", filename);
        fclose(meta);
    }

    printf("Recording to: %s\n", filename);

    while (bytes_written < total_bytes) {
        int n_read;
        if (rtlsdr_read_sync(dev, buf, BUF_SIZE, &n_read) < 0) {
            fprintf(stderr, "Read failed\n");
            break;
        }
        if ((size_t)n_read != fwrite(buf, 1, n_read, fp)) {
            perror("fwrite");
            break;
        }
        bytes_written += n_read;
    }

    printf("Done. Wrote %.2f MiB\n", bytes_written / (1024.0 * 1024.0));

    fclose(fp);
    free(buf);
    rtlsdr_close(dev);
    return 0;
}
