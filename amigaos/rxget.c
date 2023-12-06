/*
 * This file is part of MPlayer.
 *
 * MPlayer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

long rxid_get_sub_visibility()
{
	if (!mpctx) return 0;
	if (!mpctx->sh_video) return 0;

	return (mpctx->sh_video?sub_visibility:0) ;
}

long rxid_get_percent_pos()
{
	if (!mpctx) return 0;
	if (!mpctx->demuxer) return 0;

	return demuxer_get_percent_pos(mpctx->demuxer);
}

long rxid_get_time_length()
{
	if (!mpctx) return 0;
	if (!mpctx->demuxer) return 0;

	return (double)(1000.0*demuxer_get_time_length(mpctx->demuxer));
}

long rxid_get_vo_fullscreen()
{
	if (!mpctx) return 0;
	if (!mpctx->video_out) return 0;

	return  (mpctx->video_out && vo_config_count)?vo_fs:0;
}

long rxid_get_time_pos()
{
	if (!mpctx) return 0;

	if(mpctx->sh_video)
	{
		Printf("mpctx->sh_video\n");
		return (long)(1000.0*mpctx->sh_video->pts);
	}
	else if(mpctx->sh_audio && mpctx->audio_out)
	{
		Printf("mpctx->sh_audio && mpctx->audio_out\n");
		return (long)(1000.0 * (mpctx->sh_audio->stream_delay - mpctx->audio_out->get_delay() * playback_speed));
	}
	else
		return 0;
}

