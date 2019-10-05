/*
 *   IRC - Internet Relay Chat, src/modules/sapart.c
 *   (C) 2004 The UnrealIRCd Team
 *
 *   See file AUTHORS in IRC package for additional names of
 *   the programmers.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "unrealircd.h"

CMD_FUNC(cmd_sapart);

#define MSG_SAPART 	"SAPART"	

ModuleHeader MOD_HEADER
  = {
	"sapart",
	"5.0",
	"command /sapart", 
	"UnrealIRCd Team",
	"unrealircd-5",
    };

MOD_INIT()
{
	CommandAdd(modinfo->handle, MSG_SAPART, cmd_sapart, 3, CMD_USER|CMD_SERVER);
	MARK_AS_OFFICIAL_MODULE(modinfo);
	return MOD_SUCCESS;
}

MOD_LOAD()
{
	return MOD_SUCCESS;
}

MOD_UNLOAD()
{
	return MOD_SUCCESS;
}

/* cmd_sapart() - Lamego - Wed Jul 21 20:04:48 1999
   Copied off PTlink IRCd (C) PTlink coders team.
   Coded for Sadmin by Stskeeps
   also Modified by NiQuiL (niquil@programmer.net)
	parv[1] - nick to make part
	parv[2] - channel(s) to part
	parv[3] - comment
*/

CMD_FUNC(cmd_sapart)
{
	Client *target;
	Channel *channel;
	Membership *lp;
	char *name, *p = NULL;
	int i;
	char *comment = (parc > 3 && parv[3] ? parv[3] : NULL);
	char commentx[512];
	char jbuf[BUFSIZE];
	int ntargets = 0;
	int maxtargets = max_targets_for_command("SAPART");

	if (parc < 3)
        {
                sendnumeric(client, ERR_NEEDMOREPARAMS, "SAPART");
                return;
        }

        if (!(target = find_person(parv[1], NULL)))
        {
                sendnumeric(client, ERR_NOSUCHNICK, parv[1]);
                return;
        }

	/* See if we can operate on this vicim/this command */
	if (!ValidatePermissionsForPath("sacmd:sapart",client,target,NULL,NULL))
	{
		sendnumeric(client, ERR_NOPRIVILEGES);
		return;
	}

	if (MyUser(target))
	{
		/* Now works like cmd_join */
		*jbuf = 0;

		for (i = 0, name = strtoken(&p, parv[2], ","); name; name = strtoken(&p,
			NULL, ","))
		{
			if (++ntargets > maxtargets)
			{
				sendnumeric(client, ERR_TOOMANYTARGETS, name, maxtargets, "SAPART");
				break;
			}
			if (!(channel = get_channel(target, name, 0)))
			{
				sendnumeric(client, ERR_NOSUCHCHANNEL,
					name);
				continue;
			}

			/* Validate oper can do this on chan/victim */
			if (!IsULine(client) && !ValidatePermissionsForPath("sacmd:sapart",client,target,channel,NULL))
        		{
                		sendnumeric(client, ERR_NOPRIVILEGES);
				continue;
        		}
	
			if (!(lp = find_membership_link(target->user->channel, channel)))
			{
				sendnumeric(client, ERR_USERNOTINCHANNEL,
					parv[1], name);
				continue;
			}
			if (*jbuf)
				(void)strlcat(jbuf, ",", sizeof jbuf);
			(void)strlncat(jbuf, name, sizeof jbuf, sizeof(jbuf) - i - 1);
			i += strlen(name) + 1;
		}

		if (!*jbuf)
			return;

		strcpy(parv[2], jbuf);

		if (comment)
		{
			strcpy(commentx, "SAPart: ");
			strlcat(commentx, comment, 512);
		}

		parv[0] = target->name; // nick
		parv[1] = parv[2]; // chan
		parv[2] = comment ? commentx : NULL; // comment
		if (comment)
		{
			sendnotice(target,
			    "*** You were forced to part %s (%s)",
			    parv[1], commentx);
			sendto_realops("%s used SAPART to make %s part %s (%s)", client->name, target->name,
				parv[1], comment);
			sendto_server(&me, 0, 0, NULL, ":%s GLOBOPS :%s used SAPART to make %s part %s (%s)",
				me.name, client->name, target->name, parv[1], comment);
			/* Logging function added by XeRXeS */
			ircd_log(LOG_SACMDS,"SAPART: %s used SAPART to make %s part %s (%s)",
				client->name, target->name, parv[1], comment);
		}
		else
		{
			sendnotice(target,
			    "*** You were forced to part %s", parv[1]);
			sendto_realops("%s used SAPART to make %s part %s", client->name, target->name,
				parv[1]);
			sendto_server(&me, 0, 0, NULL, ":%s GLOBOPS :%s used SAPART to make %s part %s",
				me.name, client->name, target->name, parv[1]);
			/* Logging function added by XeRXeS */
			ircd_log(LOG_SACMDS,"SAPART: %s used SAPART to make %s part %s",
				client->name, target->name, parv[1]);
		}
		(void)do_cmd(target, NULL, "PART", comment ? 3 : 2, parv);
		/* target may be killed now due to the part reason @ spamfilter */
	}
	else
	{
		if (comment)
		{
			sendto_one(target, NULL, ":%s SAPART %s %s :%s", client->name,
			    parv[1], parv[2], comment);
			/* Logging function added by XeRXeS */
			ircd_log(LOG_SACMDS,"SAPART: %s used SAPART to make %s part %s (%s)",
				client->name, parv[1], parv[2], comment);
		}
		else
		{
			sendto_one(target, NULL, ":%s SAPART %s %s", client->name, parv[1],
				   parv[2]);
			/* Logging function added by XeRXeS */
			ircd_log(LOG_SACMDS,"SAPART: %s used SAPART to make %s part %s",
				client->name, parv[1], parv[2]);
		}
	}
}
