#include "SysInfoProbe.hpp"

void SysInfoProbe::GetUptimeInfo() {
	UINT64 Milliseconds = GetTickCount64();
	UINT64 Seconds = 0;
	UINT64 Minutes = 0;
	UINT64 Hours = 0;
	UINT64 Days = 0;

	Seconds = Milliseconds / 1000;
	Minutes = Seconds / 60;
	Hours = Seconds / 3600;
	if (Minutes > 1)
		Seconds -= Minutes * 60;
	
	if (Hours > 0) {
		Minutes -= Hours * 60;
		if (Hours > 24) {
			Days = Hours / 24;
			Hours -= Days * 24;
		}
	}

	Uptime.Days = Days;
	Uptime.Hours = Hours;
	Uptime.Minutes = Minutes;
	Uptime.Seconds = Seconds;
}