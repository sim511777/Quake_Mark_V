/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2009 John Fitzgibbons and others
Copyright (C) 2009-2014 Baker and others

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// cl_tent.c -- client side temporary entities

#include "quakedef.h"

static sfx_t			*cl_sfx_wizhit;
static sfx_t			*cl_sfx_knighthit;
static sfx_t			*cl_sfx_tink1;
static sfx_t			*cl_sfx_ric1;
static sfx_t			*cl_sfx_ric2;
static sfx_t			*cl_sfx_ric3;
static sfx_t			*cl_sfx_r_exp3;

/*
=================
CL_InitTEnts
=================
*/
void CL_InitTEnts (void)
{
	cl_sfx_wizhit = S_PrecacheSound ("wizard/hit.wav", NULL);
	cl_sfx_knighthit = S_PrecacheSound ("hknight/hit.wav", NULL);
	cl_sfx_tink1 = S_PrecacheSound ("weapons/tink1.wav", NULL);
	cl_sfx_ric1 = S_PrecacheSound ("weapons/ric1.wav", NULL);
	cl_sfx_ric2 = S_PrecacheSound ("weapons/ric2.wav", NULL);
	cl_sfx_ric3 = S_PrecacheSound ("weapons/ric3.wav", NULL);
	cl_sfx_r_exp3 = S_PrecacheSound ("weapons/r_exp3.wav", NULL);
}

/*
=================
CL_ParseBeam
=================
*/
void CL_ParseBeam (qmodel_t *m)
{
	int		i, ent;
	vec3_t	start, end;
	beam_t	*b;

	ent = MSG_ReadShort ();
	start[0] = MSG_ReadCoord ();
	start[1] = MSG_ReadCoord ();
	start[2] = MSG_ReadCoord ();

	end[0] = MSG_ReadCoord ();
	end[1] = MSG_ReadCoord ();
	end[2] = MSG_ReadCoord ();

	Stain_AddStain(end, 13, 22); // Intense blackening, medium radius (beam LG)

// override any beam with the same entity
	for (i = 0, b = cl.beams ; i< MAX_FITZQUAKE_BEAMS ; i++, b++)
	{
		if (b->entity == ent)
		{
			b->entity = ent;
			b->model = m;
			b->endtime = cl.time + 0.2;
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			return;
		}
	}

// find a free beam
	for (i = 0, b = cl.beams ; i< MAX_FITZQUAKE_BEAMS ; i++, b++)
	{
		if (!b->model || b->endtime < cl.time)
		{
			b->entity = ent;
			b->model = m;
			b->endtime = cl.time + 0.2;
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			return;
		}
	}

	//johnfitz -- less spammy overflow message
	if (!dev_overflows.beams || dev_overflows.beams + CONSOLE_RESPAM_TIME < realtime )
	{
		Con_Printf ("Beam list overflow!\n");
		dev_overflows.beams = realtime;
	}
	//johnfitz
}

/*
=================
CL_ParseTEnt
=================
*/
void CL_ParseTEnt (void)
{
	int		type;
	vec3_t	pos;
	dlight_t	*dl;
	int		rnd;
	int		colorStart, colorLength;

	type = MSG_ReadByte ();

	switch (type)
	{
	case TE_WIZSPIKE:			// spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();

		R_RunParticleEffect (pos, vec3_origin, 20, 30);
		Stain_AddStain(pos, 2, 20); // Minimal darkening, normal radius
		S_StartSound (-1, 0, cl_sfx_wizhit, pos, 1, 1);
		break;

	case TE_KNIGHTSPIKE:			// spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();

		R_RunParticleEffect (pos, vec3_origin, 226, 20);
		Stain_AddStain(pos, 2, 20); // Minimal darkening, normal radius
		S_StartSound (-1, 0, cl_sfx_knighthit, pos, 1, 1);
		break;

	case TE_SPIKE:			// spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();

// joe: they put the ventillator's wind effect to "10" in Nehahra. sigh.
#ifdef SUPPORTS_NEHAHRA
		if (nehahra_active)
			R_RunParticleEffect (pos, vec3_origin, 0, 9);
		else
#endif // SUPPORTS_NEHAHRA
			R_RunParticleEffect (pos, vec3_origin, 0, 10);
		Stain_AddStain(pos, 2, 20); // Minimal darkening, normal radius

		if ( rand() % 5 )
			S_StartSound (-1, 0, cl_sfx_tink1, pos, 1, 1);
		else
		{
			rnd = rand() & 3;

			if (rnd == 1)
				S_StartSound (-1, 0, cl_sfx_ric1, pos, 1, 1);
			else if (rnd == 2)
				S_StartSound (-1, 0, cl_sfx_ric2, pos, 1, 1);
			else
				S_StartSound (-1, 0, cl_sfx_ric3, pos, 1, 1);
		}

		break;

	case TE_SUPERSPIKE:			// super spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();

		R_RunParticleEffect (pos, vec3_origin, 0, 20);
		Stain_AddStain(pos, 3, 30); // Moderate darkening, moderate radius

		if ( rand() % 5 )
			S_StartSound (-1, 0, cl_sfx_tink1, pos, 1, 1);
		else
		{
			rnd = rand() & 3;

			if (rnd == 1)
				S_StartSound (-1, 0, cl_sfx_ric1, pos, 1, 1);
			else if (rnd == 2)
				S_StartSound (-1, 0, cl_sfx_ric2, pos, 1, 1);
			else
				S_StartSound (-1, 0, cl_sfx_ric3, pos, 1, 1);
		}
		break;

	case TE_GUNSHOT:			// bullet hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();

		R_RunParticleEffect (pos, vec3_origin, 0, 20);
		Stain_AddStain(pos, 2, 20); // Minimal darkening, normal radius
		break;

	case TE_EXPLOSION:			// rocket explosion
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();

		R_ParticleExplosion (pos);
		dl = CL_AllocDlight (0);
		VectorCopy (pos, dl->origin);
		dl->radius = 350;
		dl->die = cl.time + 0.5;
		dl->decay = 300;

		Stain_AddStain(pos, 5, 50); // Fair darkening, large radius
		S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		break;

	case TE_TAREXPLOSION:			// tarbaby explosion
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();

		R_BlobExplosion (pos);
		Stain_AddStain(pos, 3, 30); // Moderate darkening, moderate radius
		S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		break;

	case TE_LIGHTNING1:				// lightning bolts
		CL_ParseBeam (Mod_ForName("progs/bolt.mdl", true));
		break;

	case TE_LIGHTNING2:				// lightning bolts
		CL_ParseBeam (Mod_ForName("progs/bolt2.mdl", true));
		break;

	case TE_LIGHTNING3:				// lightning bolts
		CL_ParseBeam (Mod_ForName("progs/bolt3.mdl", true));
		break;

#ifdef SUPPORTS_NEHAHRA
	// nehahra support
	case TE_LIGHTNING4:				// lightning bolts
        CL_ParseBeam (Mod_ForName(MSG_ReadString(), true));
		break;
#endif // SUPPORTS_NEHAHRA

// PGM 01/21/97
	case TE_BEAM:				// grappling hook beam
		CL_ParseBeam (Mod_ForName("progs/beam.mdl", true));
		break;
// PGM 01/21/97

	case TE_LAVASPLASH:
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();

		R_LavaSplash (pos);
		break;

	case TE_TELEPORT:
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();

		R_TeleportSplash (pos);
		break;

	case TE_EXPLOSION2:				// color mapped explosion
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();

		colorStart = MSG_ReadByte ();
		colorLength = MSG_ReadByte ();
		R_ParticleExplosion2 (pos, colorStart, colorLength);
		dl = CL_AllocDlight (0);
		VectorCopy (pos, dl->origin);
		dl->radius = 350;
		dl->die = cl.time + 0.5;
		dl->decay = 300;

		Stain_AddStain(pos, 5, 50); // Intense darkening, intense radius
		S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		break;

#ifdef SUPPORTS_NEHAHRA
	// nehahra support
	case TE_EXPLOSION3:
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();

		R_ParticleExplosion (pos);
		dl = CL_AllocDlight (0);
		VectorCopy (pos, dl->origin);
		dl->radius = 350;
		dl->die = cl.time + 0.5;
		dl->decay = 300;

#ifdef GLQUAKE_COLORED_LIGHTS
        dl->color[0] = MSG_ReadCoord () / 2.0;
		dl->color[1] = MSG_ReadCoord () / 2.0;
		dl->color[2] = MSG_ReadCoord () / 2.0;
#else
		// Ignore the colors
		MSG_ReadCoord ();
		MSG_ReadCoord ();
		MSG_ReadCoord ();
#endif // !GLQUAKE_COLORED_LIGHTS

		Stain_AddStain(pos, 5, 50); // Intense darkening, intense radius
		S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		break;
#endif // SUPPORTS_NEHAHRA

	default:
		Host_Error ("CL_ParseTEnt: bad type");
	}
}


/*
=================
CL_NewTempEntity
=================
*/
static entity_t *CL_NewTempEntity (void)
{
	entity_t	*ent;

	if (cl.numvisedicts == MAX_MARK_V_VISEDICTS)
		return NULL;

	if (cl.num_temp_entities == MAX_FITZQUAKE_TEMP_ENTITIES)
		return NULL;

	ent = &cl.temp_entities[cl.num_temp_entities];
	memset (ent, 0, sizeof(*ent));
	cl.num_temp_entities++;
	cl.visedicts[cl.numvisedicts] = ent;
	cl.numvisedicts++;

#ifdef WINQUAKE_COLORMAP_TRANSLATION
// Baker: WinQuake only ... our default is 0 so no need for GL colored bodies
	ent->colormap = vid.colormap;
#endif // WINQUAKE_COLORMAP_TRANSLATION
	return ent;
}


/*
=================
CL_UpdateTEnts
=================
*/
void CL_UpdateTEnts (void)
{
	vec3_t		dist, org;
	float		d;
	entity_t	*ent;
	float		yaw, pitch;
	float		forward;
	int			beamnum;

	cl.num_temp_entities = 0;
	srand ((int) (cl.ctime * 1000)); //johnfitz -- freeze beams when paused

// update lightning
	for (beamnum = 0; beamnum < MAX_FITZQUAKE_BEAMS ; beamnum ++)
	{
		beam_t *b = &cl.beams[beamnum];
		if (!b->model || b->endtime < cl.time) continue;

	// if coming from the player, update the start position
		if (b->entity == cl.viewentity_player)
			VectorCopy (cl_entities[cl.viewentity_player].origin, b->start);

	// calculate pitch and yaw
		VectorSubtract (b->end, b->start, dist);

		if (dist[1] == 0 && dist[0] == 0)
		{
			// linear so pythagoras can have his coffee break
			yaw = 0;

			if (dist[2] > 0)
				pitch = 90;
			else pitch = 270;
		}
		else
		{
			yaw = (int) (atan2 (dist[1], dist[0]) * 180 / M_PI);

			if (yaw < 0) yaw += 360;

			forward = sqrt (dist[0]*dist[0] + dist[1]*dist[1]);
			pitch = (int) (atan2 (dist[2], forward) * 180 / M_PI);

			if (pitch < 0) pitch += 360;
		}

	// add new entities for the lightning
		VectorCopy (b->start, org);
		d = VectorNormalize(dist);

		// Baker: Apparently for every 30 units we do a new beam segment
		for ( ; d > 0 ; d -= 30)
		{
			ent = CL_NewTempEntity ();
			if (!ent)
				return;

			VectorCopy (org, ent->origin);
			ent->model = b->model;
			ent->angles[0] = pitch;
			ent->angles[1] = yaw;
			ent->angles[2] = rand()%360;

			// Baker: VectorMA replaces for loop
			VectorMA (org, 30, dist, org);
//			VectorCopy (org, ent->origin); // You would think, but the origin Quake loop isn't written like that

		} // End of while
	}
}

