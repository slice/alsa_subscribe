#include <alsa/asoundlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <stdbool.h>

// originally from
// http://stackoverflow.com/questions/7657624/get-master-sound-volume-in-c-in-linux
int get_audio_volume(const char* mix_name, const char* card, long* outvol) {
	int ret = 0;

	snd_mixer_t* handle;
	snd_mixer_elem_t* elem;
	snd_mixer_selem_id_t* sid;

	static int mix_index = 0;

	snd_mixer_selem_id_alloca(&sid);

	snd_mixer_selem_id_set_index(sid, mix_index);
	snd_mixer_selem_id_set_name(sid, mix_name);

	if ((snd_mixer_open(&handle, 0)) < 0)
		return -1;
	if ((snd_mixer_attach(handle, card)) < 0) {
		snd_mixer_close(handle);
		return -2;
	}
	if ((snd_mixer_selem_register(handle, NULL, NULL)) < 0) {
		snd_mixer_close(handle);
		return -3;
	}
	ret = snd_mixer_load(handle);
	if (ret < 0) {
		snd_mixer_close(handle);
		return -4;
	}
	elem = snd_mixer_find_selem(handle, sid);
	if (!elem) {
		snd_mixer_close(handle);
		return -5;
	}

	long minv, maxv;

	snd_mixer_selem_get_playback_volume_range (elem, &minv, &maxv);

	if(snd_mixer_selem_get_playback_volume(elem, 0, outvol) < 0) {
		snd_mixer_close(handle);
		return -6;
	}

	// make volume out of 100
	*outvol -= minv;
	maxv -= minv;
	minv = 0;
	*outvol = 100 * (*outvol) / maxv;

	snd_mixer_close(handle);
	return 0;
}

void print_help() {
	printf("SYNOPSIS: asub [ -m mix | -c card | -p precision | -f prefix | -h ]\n"
		"DESCRIPTION:\n"
		"\tmonitors for changes in ALSA volume and reports them to stdout\n"
		"\tby printing the new volume\n"
		"OPTIONS:\n"
		"\t-m [mix=Master]       : specify mix name\n"
		"\t-c [card=default]     : specify card name\n"
		"\t-p [precision=100000] : specify precision\n"
		"\t                        microseconds between\n"
		"\t                        checks for volume change\n"
		"\t-f [prefix]           : specfies prefix\n"
		"\t                        text printed before volume\n"
		"\t-n                    : print out volume regardless\n"
		"\t                        of change\n"
	);
}


int main(int argc, char** argv) {
	char c;
	char* opt_mix_name = "Master";
	char* opt_card = "default";
	char* opt_prefix = "";
	bool opt_continuous = false;
	long opt_precision = 100000;

	while ((c = getopt(argc, argv, "hnm:c:p:f:")) != -1) {
		switch (c) {
		case 'm':
			opt_mix_name = optarg;
			break;
		case 'c':
			opt_card = optarg;
			break;
		case 'p':
			opt_precision = strtoul(optarg, NULL, 10);
			break;
		case 'f':
			opt_prefix = optarg;
			break;
		case 'h':
			print_help();
			return 0;
		case 'n':
			opt_continuous = true;
			break;
		default:
			abort();
		}
	}

	long last_volume = -1;
	long vol = 0;

	for (;;usleep(opt_precision)) {
		get_audio_volume(opt_mix_name, opt_card, &vol);
		if (opt_continuous) {
			printf("%s%lu\n", opt_prefix, vol);
		} else {
			if (last_volume == -1) last_volume = vol;

			// this will evaluate to true
			// when the volume has changed
			if (last_volume != vol) {
				printf("%s%lu\n", opt_prefix, vol);
				last_volume = vol;
			}
		}
	}
}
