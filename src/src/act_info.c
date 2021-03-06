/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  Envy Diku Mud improvements copyright (C) 1994 by Michael Quan, David   *
 *  Love, Guilherme 'Willie' Arnold, and Mitchell Tse.                     *
 *                                                                         *
 *  EnvyMud 2.0 improvements copyright (C) 1995 by Michael Quan and        *
 *  Mitchell Tse.                                                          *
 *                                                                         *
 *  EnvyMud 2.2 improvements copyright (C) 1996, 1997 by Michael Quan.     *
 *                                                                         *
 *  In order to use any part of this Envy Diku Mud, you must comply with   *
 *  the original Diku license in 'license.doc', the Merc license in        *
 *  'license.txt', as well as the Envy license in 'license.nvy'.           *
 *  In particular, you may not remove either of these copyright notices.   *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"


char *	const	where_name	[] =
{
    "<used as light>     ",
    "<worn on finger>    ",
    "<worn on finger>    ",
    "<worn around neck>  ",
    "<worn around neck>  ",
    "<worn on body>      ",
    "<worn on head>      ",
    "<worn on legs>      ",
    "<worn on feet>      ",
    "<worn on hands>     ",
    "<worn on arms>      ",
    "<worn as shield>    ",
    "<worn about body>   ",
    "<worn about waist>  ",
    "<worn around wrist> ",
    "<worn around wrist> ",
    "<wielded, primary>  ",
    "<held>              ",
    "<wielded, second>   "
};



/*
 * Local functions.
 */
char *	format_obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch,
				       bool fShort ) );
void	show_list_to_char	args( ( OBJ_DATA *list, CHAR_DATA *ch,
				       bool fShort, bool fShowNothing ) );
void	show_char_to_char_0	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char_1	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char	args( ( CHAR_DATA *list, CHAR_DATA *ch ) );



char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort )
{
    static char buf [ MAX_STRING_LENGTH ];

    buf[0] = '\0';
    if ( IS_OBJ_STAT( obj, ITEM_INVIS)     )   strcat( buf, "(Invis) "     );
    if ( ( IS_AFFECTED( ch, AFF_DETECT_EVIL  )
	  || ( IS_SET( race_table[ ch->race ].race_abilities,
		      RACE_DETECT_ALIGN )
	      && IS_GOOD( ch ) ) )
	&& IS_OBJ_STAT( obj, ITEM_EVIL )   )   strcat( buf, "(Red Aura) "  );
    if ( IS_AFFECTED( ch, AFF_DETECT_MAGIC )
	&& IS_OBJ_STAT( obj, ITEM_MAGIC )  )   strcat( buf, "(Magical) "   );
    if ( IS_OBJ_STAT( obj, ITEM_GLOW )     )   strcat( buf, "(Glowing) "   );
    if ( IS_OBJ_STAT( obj, ITEM_HUM )      )   strcat( buf, "(Humming) "   );
    if ( IS_OBJ_STAT( obj, ITEM_POISONED ) )   strcat( buf, "(Poisoned) "  );

    if ( fShort )
    {
	if ( obj->short_descr )
	    strcat( buf, obj->short_descr );
    }
    else
    {
	if ( obj->description )
	    strcat( buf, obj->description );
    }

    return buf;
}



/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing )
{
    OBJ_DATA  *obj;
    char       buf [ MAX_STRING_LENGTH ];
    char       buf1[ MAX_STRING_LENGTH * 4 ];
    char     **prgpstrShow;
    char      *pstrShow;
    int       *prgnShow;
    int        nShow;
    int        iShow;
    int        count;
    bool       fCombine;

    if ( !ch->desc || !list )
	return;

    buf1[0] = '\0';
    /*
     * Alloc space for output lines.
     */
    count = 0;
    for ( obj = list; obj; obj = obj->next_content )
    {
	if ( obj->deleted )
	    continue;
	count++;
    }

    prgpstrShow	= alloc_mem( count * sizeof( char * ) );
    prgnShow    = alloc_mem( count * sizeof( int )    );
    nShow	= 0;

    /*
     * Format the list of objects.
     */
    for ( obj = list; obj; obj = obj->next_content )
    { 
	if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
	{
	    pstrShow = format_obj_to_char( obj, ch, fShort );
	    fCombine = FALSE;

	    if ( IS_NPC( ch ) || IS_SET( ch->act, PLR_COMBINE ) )
	    {
		/*
		 * Look for duplicates, case sensitive.
		 * Matches tend to be near end so run loop backwords.
		 */
		for ( iShow = nShow - 1; iShow >= 0; iShow-- )
		{
		    if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
		    {
			prgnShow[iShow]++;
			fCombine = TRUE;
			break;
		    }
		}
	    }

	    /*
	     * Couldn't combine, or didn't want to.
	     */
	    if ( !fCombine )
	    {
		prgpstrShow [nShow] = str_dup( pstrShow );
		prgnShow    [nShow] = 1;
		nShow++;
	    }
	}
    }

    /*
     * Output the formatted list.
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
	if ( IS_NPC( ch ) || IS_SET( ch->act, PLR_COMBINE ) )
	{
	    if ( prgnShow[iShow] != 1 )
	    {
		sprintf( buf, "(%2d) ", prgnShow[iShow] );
		strcat( buf1, buf );
	    }
	    else
	    {
		strcat( buf1, "     " );
	    }
	}
	strcat( buf1, prgpstrShow[iShow] );
	strcat( buf1, "\r\n" );
	free_string( prgpstrShow[iShow] );
    }

    if ( fShowNothing && nShow == 0 )
    {
	if ( IS_NPC( ch ) || IS_SET( ch->act, PLR_COMBINE ) )
	    strcat( buf1, "     " );
	strcat( buf1, "Nothing.\r\n" );
    }

    /*
     * Clean up.
     */
    free_mem( prgpstrShow, count * sizeof( char * ) );
    free_mem( prgnShow,    count * sizeof( int )    );

    send_to_char( buf1, ch );
    return;
}



void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf [ MAX_STRING_LENGTH ];

    buf[0] = '\0';

    if ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_WIZINVIS )
	&& get_trust( victim ) < get_trust( ch ) )
                                                strcat( buf, "(Wizinvis) "   );
    if ( IS_AFFECTED( victim, AFF_INVISIBLE )   )
                                                strcat( buf, "(Invis) "      );
    if ( IS_AFFECTED( victim, AFF_HIDE )        )
                                                strcat( buf, "(Hide) "       );
    if ( IS_AFFECTED( victim, AFF_CHARM )       )
                                                strcat( buf, "(Charmed) "    );
    if ( IS_AFFECTED( victim, AFF_PASS_DOOR )
	|| ( IS_SET( race_table[ victim->race ].race_abilities, RACE_PASSDOOR )
	    && ( !str_cmp( race_table[ victim->race ].name, "undead" )
		|| !str_cmp( race_table[ victim->race ].name, "vampire" ) ) ) )
                                                strcat( buf, "(Translucent) ");
    if ( IS_AFFECTED( victim, AFF_FAERIE_FIRE ) )
                                                strcat( buf, "(Pink Aura) "  );
    if ( IS_EVIL( victim )
	&& ( IS_AFFECTED( ch, AFF_DETECT_EVIL )
	    || ( IS_SET( race_table[ ch->race ].race_abilities,
			RACE_DETECT_ALIGN )
		&& IS_GOOD( ch ) ) ) )
                                                strcat( buf, "(Red Aura) "   );
    if ( IS_AFFECTED( victim, AFF_SANCTUARY )
	|| IS_SET( race_table[ victim->race ].race_abilities, RACE_SANCT ) )
                                                strcat( buf, "(White Aura) " );
    if ( IS_AFFECTED( victim, AFF_FLAMING ) )   strcat( buf, "(Black Aura) " );
    if ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_KILLER )  )
						strcat( buf, "(KILLER) "     );
    if ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_THIEF  )  )
						strcat( buf, "(THIEF) "      );
    if ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_REGISTER  )  )
						strcat( buf, "(PK) "      );
    if ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_AFK )  )
						strcat( buf, "(AFK) "        );

    if ( victim->position == POS_STANDING && victim->long_descr[0] != '\0' )
    {
	strcat( buf, victim->long_descr );
	send_to_char( buf, ch );
	return;
    }

    strcat( buf, PERS( victim, ch ) );
    if ( !IS_NPC( victim ) && !IS_SET( ch->act, PLR_BRIEF ) )
	strcat( buf, victim->pcdata->title );

    switch ( victim->position )
    {
    case POS_DEAD:     strcat( buf, " is DEAD!!"              ); break;
    case POS_MORTAL:   strcat( buf, " is mortally wounded."   ); break;
    case POS_INCAP:    strcat( buf, " is incapacitated."      ); break;
    case POS_STUNNED:  strcat( buf, " is lying here stunned." ); break;
    case POS_SLEEPING: strcat( buf, " is sleeping here."      ); break;
    case POS_RESTING:  strcat( buf, " is resting here."       ); break;
    case POS_STANDING: strcat( buf, " is here."               ); break;
    case POS_FIGHTING:
	strcat( buf, " is here, fighting " );
	if ( !victim->fighting )
	    strcat( buf, "thin air??" );
	else if ( victim->fighting == ch )
	    strcat( buf, "YOU!" );
	else if ( victim->in_room == victim->fighting->in_room )
	{
	    strcat( buf, PERS( victim->fighting, ch ) );
	    strcat( buf, "." );
	}
	else
	    strcat( buf, "somone who left??" );
	break;
    }

    strcat( buf, "\r\n" );
    buf[0] = UPPER( buf[0] );
    send_to_char( buf, ch );
    return;
}



void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    char      buf [ MAX_STRING_LENGTH ];
    int       iWear;
    int       percent;
    bool      found;

    if ( can_see( victim, ch ) )
    {
	act( "$n looks at you.", ch, NULL, victim, TO_VICT    );
	act( "$n looks at $N.",  ch, NULL, victim, TO_NOTVICT );
    }

    sprintf( buf, "$N the %s:\r\n\r\n", race_table[ victim->race ].name );
    act( buf, ch, NULL, victim, TO_CHAR );

    if ( victim->description[0] != '\0' )
    {
	send_to_char( victim->description, ch );
    }
    else
    {
	act( "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
    }

    if ( victim->max_hit > 0 )
	percent = ( 100 * victim->hit ) / victim->max_hit;
    else
	percent = -1;

    strcpy( buf, PERS( victim, ch ) );

         if ( percent >= 100 ) strcat( buf, " is in perfect health.\r\n"  );
    else if ( percent >=  90 ) strcat( buf, " is slightly scratched.\r\n" );
    else if ( percent >=  80 ) strcat( buf, " has a few bruises.\r\n"     );
    else if ( percent >=  70 ) strcat( buf, " has some cuts.\r\n"         );
    else if ( percent >=  60 ) strcat( buf, " has several wounds.\r\n"    );
    else if ( percent >=  50 ) strcat( buf, " has many nasty wounds.\r\n" );
    else if ( percent >=  40 ) strcat( buf, " is bleeding freely.\r\n"    );
    else if ( percent >=  30 ) strcat( buf, " is covered in blood.\r\n"   );
    else if ( percent >=  20 ) strcat( buf, " is leaking guts.\r\n"       );
    else if ( percent >=  10 ) strcat( buf, " is almost dead.\r\n"        );
    else                       strcat( buf, " is DYING.\r\n"              );

    buf[0] = UPPER( buf[0] );
    send_to_char( buf, ch );

    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( victim, iWear ) )
	    && can_see_obj( ch, obj ) )
	{
	    if ( !found )
	    {
		send_to_char( "\r\n", ch );
		act( "$E is using", ch, NULL, victim, TO_CHAR );
		found = TRUE;
	    }
	    send_to_char( where_name[iWear], ch );
	    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	    send_to_char( "\r\n", ch );
	}
    }

    if ( victim != ch
	&& !IS_NPC( ch ) && number_percent( ) < ch->pcdata->learned[gsn_peek] )
    {
	send_to_char( "\r\nYou peek at the inventory:\r\n", ch );
	show_list_to_char( victim->carrying, ch, TRUE, TRUE );
    }

    return;
}



void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch )
{
    CHAR_DATA *rch;

    if ( !list )
        return;

    for ( rch = list; rch; rch = rch->next_in_room )
    {
        if ( rch->deleted || rch == ch )
	    continue;

	if ( !IS_NPC( rch )
	    && IS_SET( rch->act, PLR_WIZINVIS )
	    && get_trust( ch ) < get_trust( rch ) )
	    continue;

	if ( can_see( ch, rch ) )
	{
	    show_char_to_char_0( rch, ch );
	}
	else if ( room_is_dark( ch->in_room )
		 && ( IS_AFFECTED( rch, AFF_INFRARED )
		     || IS_SET( race_table[ rch->race ].race_abilities,
			       RACE_INFRAVISION ) ) )
	{
	    send_to_char( "You see glowing red eyes watching YOU!\r\n", ch );
	}
    }

    return;
} 



bool check_blind( CHAR_DATA *ch )
{
    if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_HOLYLIGHT ) )
	return TRUE;

    if ( IS_AFFECTED( ch, AFF_BLIND ) )
    {
	send_to_char( "You can't see a thing!\r\n", ch );
	return FALSE;
    }

    return TRUE;
}



void do_look( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA  *obj;
    CHAR_DATA *victim;
    EXIT_DATA *pexit;
    char       buf  [ MAX_STRING_LENGTH ];
    char       arg1 [ MAX_INPUT_LENGTH  ];
    char       arg2 [ MAX_INPUT_LENGTH  ];
    char      *pdesc;
    int        door;

    if ( !IS_NPC( ch ) && !ch->desc ) 
	return;

    if ( ch->position < POS_SLEEPING )
    {
	send_to_char( "You can't see anything but stars!  See how pretty!\r\n",
		     ch );
	return;
    }

    if ( ch->position == POS_SLEEPING )
    {
	send_to_char( "You can't see anything, you're sleeping!  Zzz.\r\n",
		     ch );
	return;
    }

    if ( !check_blind( ch ) )
	return;

    if ( !IS_NPC( ch )
	&& !IS_SET( ch->act, PLR_HOLYLIGHT )
	&& room_is_dark( ch->in_room ) )
    {
	send_to_char( "It is pitch black ... \r\n", ch );
	show_char_to_char( ch->in_room->people, ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) )
    {
	/* 'look' or 'look auto' */
	send_to_char( ch->in_room->name, ch );
	send_to_char( "\r\n", ch );

	if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_AUTOEXIT ) )
	    do_exits( ch, "auto" );

	if ( arg1[0] == '\0'
	    || ( !IS_NPC( ch ) && !IS_SET( ch->act, PLR_BRIEF ) ) )
	    send_to_char( ch->in_room->description, ch );

        if ( IS_SET( ch->in_room->room_flags, ROOM_CONE_OF_SILENCE ) )
            send_to_char( "It seems eerily quiet.\r\n", ch );

	show_list_to_char( ch->in_room->contents, ch, FALSE, FALSE );
	show_char_to_char( ch->in_room->people,   ch );
	return;
    }

    if ( !str_prefix( arg1, "in" ) )
    {
	/* 'look in' */
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Look in what?\r\n", ch );
	    return;
	}

	if ( !( obj = get_obj_here( ch, arg2 ) ) )
	{
	    send_to_char( "You do not see that here.\r\n", ch );
	    return;
	}

	switch ( obj->item_type )
	{
	default:
	    send_to_char( "That is not a container.\r\n", ch );
	    break;

	case ITEM_DRINK_CON:
	    if ( obj->value[1] <= 0 )
	    {
		send_to_char( "It is empty.\r\n", ch );
		break;
	    }

	    sprintf( buf, "It's %s full of a %s liquid.\r\n",
		obj->value[1] <     obj->value[0] / 4
		    ? "less than half" :
		obj->value[1] < 3 * obj->value[0] / 4
		    ? "about half"     : "more than half",
		liq_table[obj->value[2]].liq_color
		);

	    send_to_char( buf, ch );
	    break;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    if ( IS_SET( obj->value[1], CONT_CLOSED ) )
	    {
		send_to_char( "It is closed.\r\n", ch );
		break;
	    }

	    act( "$p contains:", ch, obj, NULL, TO_CHAR );
	    show_list_to_char( obj->contains, ch, TRUE, TRUE );
	    break;
	}
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) )
    {
	show_char_to_char_1( victim, ch );
	return;
    }

    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{
	    pdesc = get_extra_descr( arg1, obj->extra_descr );
	    if ( pdesc )
	    {
		send_to_char( pdesc, ch );
		return;
	    }

	    pdesc = get_extra_descr( arg1, obj->pIndexData->extra_descr );
	    if ( pdesc )
	    {
		send_to_char( pdesc, ch );
		return;
	    }
	}

	if ( is_name( arg1, obj->name ) )
	{
	    send_to_char( obj->description, ch );
	    send_to_char( "\r\n", ch );
	    return;
	}
    }

    for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{
	    pdesc = get_extra_descr( arg1, obj->extra_descr );
	    if ( pdesc )
	    {
		send_to_char( pdesc, ch );
		return;
	    }

	    pdesc = get_extra_descr( arg1, obj->pIndexData->extra_descr );
	    if ( pdesc )
	    {
		send_to_char( pdesc, ch );
		return;
	    }
	}

	if ( is_name( arg1, obj->name ) )
	{
	    send_to_char( obj->description, ch );
	    send_to_char( "\r\n", ch );
	    return;
	}
    }

    pdesc = get_extra_descr( arg1, ch->in_room->extra_descr );
    if ( pdesc )
    {
	send_to_char( pdesc, ch );
	return;
    }

         if ( !str_prefix( arg1, "north" ) ) door = 0;
    else if ( !str_prefix( arg1, "east"  ) ) door = 1;
    else if ( !str_prefix( arg1, "south" ) ) door = 2;
    else if ( !str_prefix( arg1, "west"  ) ) door = 3;
    else if ( !str_prefix( arg1, "up"    ) ) door = 4;
    else if ( !str_prefix( arg1, "down"  ) ) door = 5;
    else
    {
	send_to_char( "You do not see that here.\r\n", ch );
	return;
    }

    /* 'look direction' */
    if ( !( pexit = ch->in_room->exit[door] ) )
    {
	send_to_char( "Nothing special there.\r\n", ch );
	return;
    }

    if ( pexit->description && pexit->description[0] != '\0' )
	send_to_char( pexit->description, ch );
    else
	send_to_char( "Nothing special there.\r\n", ch );

    if (   pexit->keyword
	&& pexit->keyword[0] != '\0'
	&& pexit->keyword[0] != ' ' )
    {
             if ( IS_SET( pexit->exit_info, EX_BASHED ) )
	    act( "The $d has been bashed from its hinges.",
		ch, NULL, pexit->keyword, TO_CHAR );
	else if ( IS_SET( pexit->exit_info, EX_CLOSED ) )
	    act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
	else if ( IS_SET( pexit->exit_info, EX_ISDOOR ) )
	    act( "The $d is open.",   ch, NULL, pexit->keyword, TO_CHAR );
    }

    return;
}



void do_examine( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char      buf [ MAX_STRING_LENGTH ];
    char      arg [ MAX_INPUT_LENGTH  ];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Examine what?\r\n", ch );
	return;
    }

    do_look( ch, arg );

    if ( ( obj = get_obj_here( ch, arg ) ) )
    {
	switch ( obj->item_type )
	{
	default:
	    break;

	case ITEM_DRINK_CON:
	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    send_to_char( "When you look inside, you see:\r\n", ch );
	    sprintf( buf, "in %s", arg );
	    do_look( ch, buf );
	}
    }

    return;
}



/*
 * Thanks to Zrin for auto-exit part.
 */
void do_exits( CHAR_DATA *ch, char *argument )
{
           EXIT_DATA       *pexit;
    extern char *    const  dir_name [ ];
           char             buf      [ MAX_STRING_LENGTH ];
           int              door;
           bool             found;
           bool             fAuto;

    buf[0] = '\0';
    fAuto  = !str_cmp( argument, "auto" );

    if ( !check_blind( ch ) )
	return;

    strcpy( buf, fAuto ? "[Exits:" : "Obvious exits:\r\n" );

    found = FALSE;
    for ( door = 0; door <= 5; door++ )
    {
	if ( ( pexit = ch->in_room->exit[door] )
	    && pexit->to_room
	    && !IS_SET( pexit->exit_info, EX_CLOSED ) )
	{
	    found = TRUE;
	    if ( fAuto )
	    {
		strcat( buf, " " );
		strcat( buf, dir_name[door] );
	    }
	    else
	    {
		sprintf( buf + strlen( buf ), "%-5s - %s\r\n",
		    capitalize( dir_name[door] ),
		    room_is_dark( pexit->to_room )
			?  "Too dark to tell"
			: pexit->to_room->name
		    );
	    }
	}
    }

    if ( !found )
	strcat( buf, fAuto ? " none" : "None.\r\n" );

    if ( fAuto )
	strcat( buf, "]\r\n" );

    send_to_char( buf, ch );
    return;
}



void do_score( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA *paf;
    char         buf  [ MAX_STRING_LENGTH ];
    char         buf1 [ MAX_STRING_LENGTH ];

    buf1[0] = '\0';
    sprintf( buf,
	    "You are %s%s, level %d, %d years old (%d hours).\r\n",
	    ch->name,
	    IS_NPC( ch ) ? "" : ch->pcdata->title,
	    ch->level,
	    get_age( ch ),
	    ( ch->played / 3600 ) );
    strcat( buf1, buf );

    sprintf( buf, "You are %s.\r\n", race_table[ ch->race ].name );
    strcat( buf1, buf );

    if ( get_trust( ch ) != ch->level )
    {
	sprintf( buf, "You are trusted at level %d.\r\n",
		get_trust( ch ) );
	strcat( buf1, buf );
    }

    sprintf( buf,
	"You have %d/%d hit, %d/%d mana, %d/%d movement, %d practices.\r\n",
	    ch->hit,  ch->max_hit,
	    ch->mana, ch->max_mana,
	    ch->move, ch->max_move,
	    ch->practice );
    strcat( buf1, buf );

    sprintf( buf,
	    "You are carrying %d/%d items with weight %d/%d kg.\r\n",
	    ch->carry_number, can_carry_n( ch ),
	    ch->carry_weight, can_carry_w( ch ) );
    strcat( buf1, buf );

    sprintf( buf,
	    "Str: %d  Int: %d  Wis: %d  Dex: %d  Con: %d.\r\n",
	    get_curr_str( ch ),
	    get_curr_int( ch ),
	    get_curr_wis( ch ),
	    get_curr_dex( ch ),
	    get_curr_con( ch ) );
    strcat( buf1, buf );

    sprintf( buf,
	    "You have scored %d exp, and have %d gold coins.\r\n",
	    ch->exp,  ch->gold );
    strcat( buf1, buf );

    sprintf( buf,
	    "Autoexit: %s.  Autoloot: %s.  Autogold: %s.  Autosac: %s.\r\n",
	    ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_AUTOEXIT ) ) ? "yes"
	                                                         : "no",
	    ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_AUTOLOOT ) ) ? "yes"
	                                                         : "no",
	    ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_AUTOGOLD ) ) ? "yes"
	                                                         : "no",
	    ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_AUTOSAC  ) ) ? "yes"
	                                                         : "no" );
    strcat( buf1, buf );
    
    sprintf( buf, "Wimpy set to %d hit points.\r\n", ch->wimpy );
    strcat( buf1, buf );

    if ( !IS_NPC( ch ) )
    {
	sprintf( buf, "Page pausing set to %d lines of text.\r\n",
		ch->pcdata->pagelen );
	strcat( buf1, buf );
    }

    if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK]   > 10 )
	strcat( buf1, "You are drunk.\r\n"   );
    if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_THIRST] ==  0
	&& ch->level < LEVEL_IMMORTAL )
	strcat( buf1, "You are thirsty.\r\n" );
    if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_FULL]   ==  0
	&& ch->level < LEVEL_IMMORTAL )
	strcat( buf1, "You are hungry.\r\n"  );

    switch ( ch->position )
    {
    case POS_DEAD:     
	strcat( buf1, "You are DEAD!!\r\n"            ); break;
    case POS_MORTAL:
	strcat( buf1, "You are mortally wounded.\r\n" ); break;
    case POS_INCAP:
	strcat( buf1, "You are incapacitated.\r\n"    ); break;
    case POS_STUNNED:
	strcat( buf1, "You are stunned.\r\n"          ); break;
    case POS_SLEEPING:
	strcat( buf1, "You are sleeping.\r\n"         ); break;
    case POS_RESTING:
	strcat( buf1, "You are resting.\r\n"          ); break;
    case POS_STANDING:
	strcat( buf1, "You are standing.\r\n"         ); break;
    case POS_FIGHTING:
	strcat( buf1, "You are fighting.\r\n"         ); break;
    }

    if ( ch->level >= 25 )
    {
	sprintf( buf, "AC: %d.  ", GET_AC( ch ) );
	strcat( buf1, buf );
    }

    strcat( buf1, "You are " );
         if ( GET_AC( ch ) >=  101 ) strcat( buf1, "WORSE than naked!\r\n" );
    else if ( GET_AC( ch ) >=   80 ) strcat( buf1, "naked.\r\n"            );
    else if ( GET_AC( ch ) >=   60 ) strcat( buf1, "wearing clothes.\r\n"  );
    else if ( GET_AC( ch ) >=   40 ) strcat( buf1, "slightly armored.\r\n" );
    else if ( GET_AC( ch ) >=   20 ) strcat( buf1, "somewhat armored.\r\n" );
    else if ( GET_AC( ch ) >=    0 ) strcat( buf1, "armored.\r\n"          );
    else if ( GET_AC( ch ) >= - 20 ) strcat( buf1, "well armored.\r\n"     );
    else if ( GET_AC( ch ) >= - 40 ) strcat( buf1, "strongly armored.\r\n" );
    else if ( GET_AC( ch ) >= - 60 ) strcat( buf1, "heavily armored.\r\n"  );
    else if ( GET_AC( ch ) >= - 80 ) strcat( buf1, "superbly armored.\r\n" );
    else if ( GET_AC( ch ) >= -100 ) strcat( buf1, "divinely armored.\r\n" );
    else                           strcat( buf1, "invincible!\r\n"       );

    if ( ch->level >= 15 )
    {
        if ( IS_NPC( ch )
	    || ch->level >=
                skill_table[gsn_dual].skill_level[ch->class] )
            strcat ( buf1, "Primary Weapon " );
        sprintf( buf, "Hitroll: %d  Damroll: %d.\r\n",
            get_hitroll( ch, WEAR_WIELD ), get_damroll( ch, WEAR_WIELD) );
        strcat( buf1, buf );
        if ( get_eq_char( ch, WEAR_WIELD_2 ) )
        {
            sprintf( buf, " Second Weapon Hitroll: %d  Damroll: %d.\r\n",
		get_hitroll( ch, WEAR_WIELD_2 ),
		get_damroll( ch, WEAR_WIELD_2 ) );
            strcat( buf1, buf );
        }
    }
    
    if ( ch->level >= 10 )
    {
	sprintf( buf, "Alignment: %d.  ", ch->alignment );
	strcat( buf1, buf );
    }

    strcat( buf1, "You are " );
         if ( ch->alignment >  900 ) strcat( buf1, "angelic.\r\n" );
    else if ( ch->alignment >  700 ) strcat( buf1, "saintly.\r\n" );
    else if ( ch->alignment >  350 ) strcat( buf1, "good.\r\n"    );
    else if ( ch->alignment >  100 ) strcat( buf1, "kind.\r\n"    );
    else if ( ch->alignment > -100 ) strcat( buf1, "neutral.\r\n" );
    else if ( ch->alignment > -350 ) strcat( buf1, "mean.\r\n"    );
    else if ( ch->alignment > -700 ) strcat( buf1, "evil.\r\n"    );
    else if ( ch->alignment > -900 ) strcat( buf1, "demonic.\r\n" );
    else                             strcat( buf1, "satanic.\r\n" );
    
    if ( ch->affected )
    {
        bool printed = FALSE;

	for ( paf = ch->affected; paf; paf = paf->next )
	{
	    if ( paf->deleted )
	        continue;

	    if ( !printed )
	    {
		strcat( buf1, "You are affected by:\r\n" );
		printed = TRUE;
	    }

	    sprintf( buf, "Spell: '%s'", skill_table[paf->type].name );
	    strcat( buf1, buf );

	    if ( ch->level >= 20 )
	    {
		sprintf( buf,
			" modifies %s by %d for %d hours",
			affect_loc_name( paf->location ),
			paf->modifier,
			paf->duration );
		strcat( buf1, buf );
	    }

	    strcat( buf1, ".\r\n" );
	}
    }

    send_to_char( buf1, ch );
    return;
}



char *	const	day_name	[] =
{
    "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
    "the Great Gods", "the Sun"
};

char *	const	month_name	[] =
{
    "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
    "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
    "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
    "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

void do_time( CHAR_DATA *ch, char *argument )
{
           char  buf           [ MAX_STRING_LENGTH ];
    extern char  str_boot_time[];
           char *suf;
           int   day;

    day     = time_info.day + 1;

         if ( day > 4 && day <  20 ) suf = "th";
    else if ( day % 10 ==  1       ) suf = "st";
    else if ( day % 10 ==  2       ) suf = "nd";
    else if ( day % 10 ==  3       ) suf = "rd";
    else                             suf = "th";

    sprintf( buf,
	    "It is %d o'clock %s, Day of %s, %d%s the Month of %s.\r\n",
	    ( time_info.hour % 12 == 0 ) ? 12 : time_info.hour % 12,
	    time_info.hour >= 12 ? "pm" : "am",
	    day_name[day % 7],
	    day, suf,
	    month_name[time_info.month] );
    send_to_char( buf, ch );
    sprintf( buf,
	    "EnvyMud started up %s\rThe system time is %s\r",
	    str_boot_time,
	    (char *) ctime( &current_time ) );
    send_to_char( buf, ch );
    if ( down_time > 0 )
    {
	sprintf( buf, "%s will be at %s\r", Reboot ? "Reboot" : "Shutdown",
		(char *) ctime( &down_time ) );
	send_to_char( buf, ch );
    }
    return;
}



void do_weather( CHAR_DATA *ch, char *argument )
{
           char         buf     [ MAX_STRING_LENGTH ];
    static char * const sky_look[ 4 ] =
    {
	"cloudless",
	"cloudy",
	"rainy",
	"lit by flashes of lightning"
    };

    if ( !IS_OUTSIDE( ch ) )
    {
	send_to_char( "You can't see the weather indoors.\r\n", ch );
	return;
    }

    sprintf( buf, "The sky is %s and %s.\r\n",
	    sky_look[weather_info.sky],
	    weather_info.change >= 0
	    ? "a warm southerly breeze blows"
	    : "a cold northern gust blows" );
    send_to_char( buf, ch );
    return;
}



void do_help( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;
    bool       hfound = FALSE; /* modification to show multiple helps
				  by Robin Goodall from merc mailing list */

    if ( argument[0] == '\0' )
	argument = "summary";

    for ( pHelp = help_first; pHelp; pHelp = pHelp->next )
    {
	if ( pHelp->level > get_trust( ch ) )
	    continue;

	if ( is_name( argument, pHelp->keyword ) )
	{
	    if ( hfound )
	        send_to_char( "\r\n----------------------------------------------------------------------------\r\n\r\n", ch );

	    if ( pHelp->level >= 0 && str_cmp( argument, "imotd" ) )
	    {
		send_to_char( pHelp->keyword, ch );
		send_to_char( "\r\n", ch );
	    }

	    /*
	     * Strip leading '.' to allow initial blanks.
	     */
	    if ( pHelp->text[0] == '.' )
		send_to_char( pHelp->text+1, ch );
	    else
		send_to_char( pHelp->text  , ch );
	    hfound = TRUE;
	}
    }
    
    if ( !hfound )
        send_to_char( "No help on that word.\r\n", ch );
    return;
}



/*
 * New 'who' command originally by Alander of Rivers of Mud.
 */
void do_who( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    char             buf      [ MAX_STRING_LENGTH*3 ];
    char             buf2     [ MAX_STRING_LENGTH   ];
    int              iClass;
    int              iLevelLower;
    int              iLevelUpper;
    int              nNumber;
    int              nMatch;
    bool             rgfClass [ MAX_CLASS ];
    bool             fClassRestrict;
    bool             fImmortalOnly;
 
    /*
     * Set default arguments.
     */
    iLevelLower    = 0;
    iLevelUpper    = L_DIR; /*Used to be Max_level */
    fClassRestrict = FALSE;
    fImmortalOnly  = FALSE;
    for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	rgfClass[iClass] = FALSE;

    /*
     * Parse arguments.
     */
    nNumber = 0;
    for ( ;; )
    {
	char arg [ MAX_STRING_LENGTH ];

	argument = one_argument( argument, arg );
	if ( arg[0] == '\0' )
	    break;

	if ( is_number( arg ) )
	{
	    switch ( ++nNumber )
	    {
	    case 1: iLevelLower = atoi( arg ); break;
	    case 2: iLevelUpper = atoi( arg ); break;
	    default:
		send_to_char( "Only two level numbers allowed.\r\n", ch );
		return;
	    }
	}
	else
	{
	    int iClass;

	    if ( strlen( arg ) < 3 )
	    {
		send_to_char( "Classes must be longer than that.\r\n", ch );
		return;
	    }

	    /*
	     * Look for classes to turn on.
	     */
	    arg[3]    = '\0';
	    if ( !str_cmp( arg, "imm" ) )
	    {
		fImmortalOnly = TRUE;
	    }
	    else
	    {
		fClassRestrict = TRUE;
		for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
		{
		    if ( !str_cmp( arg, class_table[iClass].who_name ) )
		    {
			rgfClass[iClass] = TRUE;
			break;
		    }
		}

		if ( iClass == MAX_CLASS )
		{
		    send_to_char( "That's not a class.\r\n", ch );
		    return;
		}
	    }
	}
    }

    /*
     * Now show matching chars.
     */
    nMatch = 0;
    buf[0] = '\0';
    for ( d = descriptor_list; d; d = d->next )
    {
	CHAR_DATA       *wch;
	char      const *class;

	wch   = ( d->original ) ? d->original : d->character;

	/*
	 * Check for match against restrictions.
	 * Don't use trust as that exposes trusted mortals.
	 */
	if ( d->connected != CON_PLAYING || !can_see( ch, wch ) )
	    continue;

	if (   wch->level < iLevelLower
	    || wch->level > iLevelUpper
	    || ( fImmortalOnly  && wch->level < LEVEL_HERO )
	    || ( fClassRestrict && !rgfClass[wch->class] ) )
	    continue;

	nMatch++;

	/*
	 * Figure out what to print for class.
	 */
	class = class_table[wch->class].who_name;
	if ( wch->level >= LEVEL_IMMORTAL )
	    switch ( wch->level )
	      {
	      default: break;
	      case L_DIR: class = "DIRECT"; break;
	      case L_SEN: class = "SENIOR"; break;
	      case L_JUN: class = "JUNIOR"; break;
	      case L_APP: class = "APPREN"; break;
	      }

	/*
	 * Format it up.
	 */
	if ( wch->level < LEVEL_IMMORTAL )
	    sprintf( buf + strlen( buf ), "(%2d %s %-10s) %s%s%s%s%s%s\r\n",
		    wch->level,
		    class,
		    race_table[wch->race].name,
		    IS_SET( wch->act, PLR_KILLER   ) ? "(KILLER) " : "",
		    IS_SET( wch->act, PLR_THIEF    ) ? "(THIEF) "  : "",
		    IS_SET( wch->act, PLR_REGISTER ) ? "(PK) "     : "",
		    IS_SET( wch->act, PLR_AFK      ) ? "(AFK) "    : "",
		    wch->name,
		    wch->pcdata->title );
	else
	    sprintf( buf + strlen( buf ), "(      %s     ) %s%s%s%s%s%s%s\r\n",
		    class,
		    IS_SET( wch->act, PLR_WIZINVIS ) ? "(WIZINVIS) " : "", 
		    IS_SET( wch->act, PLR_KILLER   ) ? "(KILLER) " : "",
		    IS_SET( wch->act, PLR_THIEF    ) ? "(THIEF) "  : "",
		    IS_SET( wch->act, PLR_REGISTER ) ? "(PK) "     : "",
		    IS_SET( wch->act, PLR_AFK      ) ? "(AFK) "    : "",
		    wch->name,
		    wch->pcdata->title );
    }

    sprintf( buf2, "You see %d player%s in the game.\r\n",
	    nMatch, nMatch == 1 ? "" : "s" );
    strcat( buf, buf2 );
    send_to_char( buf, ch );
    return;
}

/* Contributed by Kaneda */
void do_whois( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    char             buf  [ MAX_STRING_LENGTH  ];
    char             name [ MAX_INPUT_LENGTH   ];
 
    one_argument( argument, name );

    if( name[0] == '\0' )
    {
	send_to_char( "Usage:  whois <name>\r\n", ch );
	return;
    }

    name[0] = UPPER( name[0] );

    buf[0] = '\0';
    for( d = descriptor_list ; d ; d = d->next )
    {
	CHAR_DATA       *wch;
	char      const *class;
	
	wch = ( d->original ) ? d->original : d->character; 

	if( d->connected != CON_PLAYING || !can_see( ch, wch ) )
	    continue;
  
	if( str_prefix( name, wch->name ) )
	    continue;

	class = class_table[ wch->class ].who_name;
	if( wch->level >= LEVEL_IMMORTAL )
	    switch( wch->level )
	    {
	      case L_DIR: class = "DIRECT"; break;
	      case L_SEN: class = "SENIOR"; break;
	      case L_JUN: class = "JUNIOR"; break;
	      case L_APP: class = "APPREN"; break;
	    }
    
	/*
	 * Format it up.
	 */
	if ( wch->level < LEVEL_IMMORTAL )
	    sprintf( buf + strlen( buf ), "(%2d %s %-10s) %s%s%s%s%s%s\r\n",
		    wch->level,
		    class,
		    race_table[wch->race].name,
		    IS_SET( wch->act, PLR_KILLER   ) ? "(KILLER) " : "",
		    IS_SET( wch->act, PLR_THIEF    ) ? "(THIEF) "  : "",
		    IS_SET( wch->act, PLR_REGISTER ) ? "(PK) "     : "",
		    IS_SET( wch->act, PLR_AFK      ) ? "(AFK) "    : "",
		    wch->name,
		    wch->pcdata->title );
	else
	    sprintf( buf + strlen( buf ), "(      %s     ) %s%s%s%s%s%s%s\r\n",
		    class,
		    IS_SET( wch->act, PLR_WIZINVIS ) ? "(WIZINVIS) " : "", 
		    IS_SET( wch->act, PLR_KILLER   ) ? "(KILLER) " : "",
		    IS_SET( wch->act, PLR_THIEF    ) ? "(THIEF) "  : "",
		    IS_SET( wch->act, PLR_REGISTER ) ? "(PK) "     : "",
		    IS_SET( wch->act, PLR_AFK      ) ? "(AFK) "    : "",
		    wch->name,
		    wch->pcdata->title );
      }

    if ( buf[0] == '\0' )
        send_to_char( "No one matches the given criteria.\r\n", ch );
    else
        send_to_char( buf, ch );
    return;
}


void do_inventory( CHAR_DATA *ch, char *argument )
{
    send_to_char( "You are carrying:\r\n", ch );
    show_list_to_char( ch->carrying, ch, TRUE, TRUE );
    return;
}



void do_equipment( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int       iWear;
    bool      found;

    send_to_char( "You are using:\r\n", ch );
    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( !( obj = get_eq_char( ch, iWear ) ) )
	    continue;

	send_to_char( where_name[iWear], ch );
	if ( can_see_obj( ch, obj ) )
	{
	    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	    send_to_char( "\r\n", ch );
	}
	else
	{
	    send_to_char( "something.\r\n", ch );
	}
	found = TRUE;
    }

    if ( !found )
	send_to_char( "Nothing.\r\n", ch );

    return;
}



void do_compare( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj1;
    OBJ_DATA *obj2;
    char     *msg;
    char      arg1 [ MAX_INPUT_LENGTH ];
    char      arg2 [ MAX_INPUT_LENGTH ];
    int       value1;
    int       value2;

    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Compare what to what?\r\n", ch );
	return;
    }

    if ( !( obj1 = get_obj_carry( ch, arg1 ) ) )
    {
	send_to_char( "You do not have that item.\r\n", ch );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	for ( obj2 = ch->carrying; obj2; obj2 = obj2->next_content )
	{
	    if ( obj2->wear_loc != WEAR_NONE
		&& can_see_obj( ch, obj2 )
		&& obj1->item_type == obj2->item_type
		&& ( obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0 )
		break;
	}

	if ( !obj2 )
	{
	    send_to_char( "You aren't wearing anything comparable.\r\n", ch );
	    return;
	}
    }
    else
    {
	if ( !( obj2 = get_obj_carry( ch, arg2 ) ) )
	{
	    char new_arg2 [ MAX_INPUT_LENGTH ];
	    int  number;

	    /*  Strip off number argument, subtract one, paste it together */
	    number = number_argument( arg2, arg2 );
	    if ( number > 1 )  number--;
	    sprintf( new_arg2, "%d.%s", number, arg2 );

	    if ( !( obj2 = get_obj_wear( ch, new_arg2 ) ) )
	    {
		send_to_char( "You do not have that item.\r\n", ch );
		return;
	    }
	
	    if ( ( obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE ) == 0 )
	    {
		send_to_char( "They are not comparable items.\r\n", ch );
		return;
	    }

	}
    }
	    
    msg		= NULL;
    value1	= 0;
    value2	= 0;

    if ( obj1 == obj2 )
    {
	msg = "You compare $p to itself.  It looks about the same.";
    }
    else if ( obj1->item_type != obj2->item_type )
    {
	msg = "You can't compare $p and $P.";
    }
    else
    {
	switch ( obj1->item_type )
	{
	default:
	    msg = "You can't compare $p and $P.";
	    break;

	case ITEM_ARMOR:
	    value1 = obj1->value[0];
	    value2 = obj2->value[0];
	    break;

	case ITEM_WEAPON:
	    value1 = obj1->value[1] + obj1->value[2];
	    value2 = obj2->value[1] + obj2->value[2];
	    break;
	}
    }

    if ( !msg )
    {
        if ( obj2->wear_loc != WEAR_NONE )
        {
                 if ( value1 == value2 )
                     msg = "$p and $P (equipped) look about the same.";
           else if ( value1  > value2 )
                     msg = "$p looks better than $P (equipped).";
            else
                     msg = "$p looks worse than $P (equipped).";
        }
        else
        {
             if ( value1 == value2 ) msg = "$p and $P look about the same.";
        else if ( value1  > value2 ) msg = "$p looks better than $P.";
        else                         msg = "$p looks worse than $P.";
        }

    }

    act( msg, ch, obj1, obj2, TO_CHAR );
    return;
}



void do_credits( CHAR_DATA *ch, char *argument )
{
    do_help( ch, "diku" );
    return;
}



void do_where( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA       *victim;
    DESCRIPTOR_DATA *d;
    char             buf [ MAX_STRING_LENGTH ];
    char             arg [ MAX_INPUT_LENGTH  ];
    bool             found;

    if ( !check_blind( ch ) )
        return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Players near you:\r\n", ch );
	found = FALSE;
	for ( d = descriptor_list; d; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
		&& ( victim = d->character )
		&& !IS_NPC( victim )
		&& victim->in_room
		&& victim->in_room->area == ch->in_room->area
		&& can_see( ch, victim ) )
	    {
		found = TRUE;
		sprintf( buf, "%-28s %s\r\n",
			victim->name, victim->in_room->name );
		send_to_char( buf, ch );
	    }
	}
	if ( !found )
	    send_to_char( "None\r\n", ch );
    }
    else
    {
	found = FALSE;
	for ( victim = char_list; victim; victim = victim->next )
	{
	    if ( !victim->in_room
		|| IS_AFFECTED( victim, AFF_HIDE ) 
		|| IS_AFFECTED( victim, AFF_SNEAK ) )
	        continue;

	    if ( victim->in_room->area == ch->in_room->area
		&& can_see( ch, victim )
		&& is_name( arg, victim->name ) )
	    {
		found = TRUE;
		sprintf( buf, "%-28s %s\r\n",
			PERS( victim, ch ), victim->in_room->name );
		send_to_char( buf, ch );
		break;
	    }
	}
	if ( !found )
	    act( "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
    }

    return;
}




void do_consider( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char      *buf                      = '\0';
    char      *msg;
    char       arg [ MAX_INPUT_LENGTH ];
    int        diff;
    int        hpdiff;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Consider killing whom?\r\n", ch );
	return;
    }

    if ( !( victim = get_char_room( ch, arg ) ) )
    {
	send_to_char( "They're not here.\r\n", ch );
	return;
    }

    if ( !IS_NPC( victim ) && !IS_SET( victim->act, PLR_KILLER ) )
    {
	send_to_char( "The gods do not accept this type of sacrifice.\r\n",
		     ch );
	return;
    }

    diff = victim->level - ch->level;

         if ( diff <= -10 ) msg = "You can kill $N naked and weaponless.";
    else if ( diff <=  -5 ) msg = "$N is no match for you.";
    else if ( diff <=  -2 ) msg = "$N looks like an easy kill.";
    else if ( diff <=   1 ) msg = "The perfect match!";
    else if ( diff <=   4 ) msg = "$N says 'Do you feel lucky, punk?'.";
    else if ( diff <=   9 ) msg = "$N laughs at you mercilessly.";
    else                    msg = "Death will thank you for your gift.";

    act( msg, ch, NULL, victim, TO_CHAR );

    /* additions by king@tinuviel.cs.wcu.edu */
    hpdiff = ( ch->hit - victim->hit );

    if ( ( ( diff >= 0) && ( hpdiff <= 0 ) )
	|| ( ( diff <= 0 ) && ( hpdiff >= 0 ) ) )
    {
        send_to_char( "Also,", ch );
    }
    else
    {
        send_to_char( "However,", ch );
    }

    if ( hpdiff >= 101 )
        buf = " you are currently much healthier than $E.";
    if ( hpdiff <= 100 )
        buf = " you are currently healthier than $E.";
    if ( hpdiff <= 50 ) 
        buf = " you are currently slightly healthier than $E.";
    if ( hpdiff <= 25 )
        buf = " you are a teensy bit healthier than $E.";
    if ( hpdiff <= 0 )
        buf = " $E is a teensy bit healthier than you.";
    if ( hpdiff <= -25 )
        buf = " $E is slightly healthier than you.";
    if ( hpdiff <= -50 )
        buf = " $E is healthier than you.";
    if ( hpdiff <= -100 )
        buf = " $E is much healthier than you.";

    act( buf, ch, NULL, victim, TO_CHAR );
    return;
}



void set_title( CHAR_DATA *ch, char *title )
{
    char buf [ MAX_STRING_LENGTH ];

    if ( IS_NPC( ch ) )
    {
	bug( "Set_title: NPC.", 0 );
	return;
    }

    buf[0] = '\0';

    if ( isalpha( title[0] ) || isdigit( title[0] ) )
    {
	buf[0] = ' ';
	strcpy( buf+1, title );
    }
    else
    {
	strcpy( buf, title );
    }

    free_string( ch->pcdata->title );
    ch->pcdata->title = str_dup( buf );
    return;
}



void do_title( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch ) )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Change your title to what?\r\n", ch );
	return;
    }

    if ( strlen( argument ) > 50 )
	argument[50] = '\0';

    smash_tilde( argument );
    set_title( ch, argument );
    send_to_char( "Ok.\r\n", ch );
}



void do_description( CHAR_DATA *ch, char *argument )
{
    char buf [ MAX_STRING_LENGTH ];

    if ( IS_NPC( ch ) )
        return;

    if ( argument[0] != '\0' )
    {
	buf[0] = '\0';
	smash_tilde( argument );
	if ( argument[0] == '+' )
	{
	    if ( ch->description )
		strcat( buf, ch->description );
	    argument++;
	    while ( isspace( *argument ) )
		argument++;
	}

	if ( strlen( buf ) + strlen( argument ) >=  MAX_STRING_LENGTH  - 2 )
	{
	    send_to_char( "Description too long.\r\n", ch );
	    return;
	}

	strcat( buf, argument );
	strcat( buf, "\r\n" );
	free_string( ch->description );
	ch->description = str_dup( buf );
    }

    send_to_char( "Your description is:\r\n", ch );
    send_to_char( ch->description ? ch->description : "(None).\r\n", ch );
    return;
}



void do_report( CHAR_DATA *ch, char *argument )
{
    char buf [ MAX_INPUT_LENGTH ];

    sprintf( buf,
	    "You report: %d/%d hp %d/%d mana %d/%d mv %d xp.\r\n",
	    ch->hit,  ch->max_hit,
	    ch->mana, ch->max_mana,
	    ch->move, ch->max_move,
	    ch->exp );

    send_to_char( buf, ch );

    sprintf( buf,
	    "$n reports: %d/%d hp %d/%d mana %d/%d mv %d xp.",
	    ch->hit,  ch->max_hit,
	    ch->mana, ch->max_mana,
	    ch->move, ch->max_move,
	    ch->exp );

    act( buf, ch, NULL, NULL, TO_ROOM );

    return;
}



void do_practice( CHAR_DATA *ch, char *argument )
{
    char buf  [ MAX_STRING_LENGTH   ];
    char buf1 [ MAX_STRING_LENGTH*2 ];
    int  sn;
    int  money = ch->level * ch->level * 20;

    if ( IS_NPC( ch ) )
	return;

    buf1[0] = '\0';

    if ( ch->level < 3 )
    {
	send_to_char(
	    "You must be third level to practice.  Go train instead!\r\n",
	    ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	CHAR_DATA *mob;
	int        col;

	for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
	{
	    if ( mob->deleted )
	        continue;
	    if ( IS_NPC( mob ) && IS_SET( mob->act, ACT_PRACTICE ) )
	        break;
	}

	col    = 0;
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( !skill_table[sn].name )
		break;
	    if ( ch->level < skill_table[sn].skill_level[ch->class]
		|| skill_table[sn].skill_level[ch->class] > LEVEL_HERO )
		continue;

	    if ( ( mob ) || ( ch->pcdata->learned[sn] > 0 ) )
	    {
		sprintf( buf, "%18s %3d%%  ",
			skill_table[sn].name, ch->pcdata->learned[sn] );
		strcat( buf1, buf );
		if ( ++col % 3 == 0 )
		    strcat( buf1, "\r\n" );
	    }
	}

	if ( col % 3 != 0 )
	    strcat( buf1, "\r\n" );

	sprintf( buf, "You have %d practice sessions left.\r\n",
		ch->practice );
	strcat( buf1, buf );
	sprintf( buf, "Cost of practicing is %d gold coins.\r\n", money );
	strcat( buf1, buf );
	send_to_char( buf1, ch );
    }
    else
    {
	CHAR_DATA *mob;
	int        adept;

	if ( !IS_AWAKE( ch ) )
	{
	    send_to_char( "In your dreams, or what?\r\n", ch );
	    return;
	}

	for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
	{
	    if ( mob->deleted )
	        continue;
	    if ( IS_NPC( mob ) && IS_SET( mob->act, ACT_PRACTICE ) )
		break;
	}

	if ( !mob )
	{
	    send_to_char( "You can't do that here.\r\n", ch );
	    return;
	}

	if ( ch->practice <= 0 )
	{
	    send_to_char( "You have no practice sessions left.\r\n", ch );
	    return;
	}
	else if ( money > ch->gold )
	{
	    send_to_char( "You don't have enough money to practice.\r\n", ch );
	    return;
	}

	if ( ( sn = skill_lookup( argument ) ) < 0
	    || ( !IS_NPC( ch )
		&& ch->level < skill_table[sn].skill_level[ch->class] ) )
	{
	    send_to_char( "You can't practice that.\r\n", ch );
	    return;
	}

	adept = IS_NPC( ch ) ? 100 : class_table[ch->class].skill_adept;

	if ( ch->pcdata->learned[sn] >= adept )
	{
	    sprintf( buf, "You are already an adept of %s.\r\n",
		skill_table[sn].name );
	    send_to_char( buf, ch );
	}
	else
	{
	    ch->practice--;
	    ch->gold                -= money;
	    ch->pcdata->learned[sn] += int_app[get_curr_int( ch )].learn;
	    if ( ch->pcdata->learned[sn] < adept )
	    {
		act( "You practice $T.",
		    ch, NULL, skill_table[sn].name, TO_CHAR );
		act( "$n practices $T.",
		    ch, NULL, skill_table[sn].name, TO_ROOM );
	    }
	    else
	    {
		ch->pcdata->learned[sn] = adept;
		act( "You are now an adept of $T.",
		    ch, NULL, skill_table[sn].name, TO_CHAR );
		act( "$n is now an adept of $T.",
		    ch, NULL, skill_table[sn].name, TO_ROOM );
	    }
	}
    }
    return;
}



/*
 * 'Wimpy' originally by Dionysos.
 */
void do_wimpy( CHAR_DATA *ch, char *argument )
{
    char buf [ MAX_STRING_LENGTH ];
    char arg [ MAX_INPUT_LENGTH  ];
    int  wimpy;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
	wimpy = ch->max_hit / 5;
    else
	wimpy = atoi( arg );

    if ( wimpy < 0 )
    {
	send_to_char( "Your courage exceeds your wisdom.\r\n", ch );
	return;
    }

    if ( wimpy > ch->max_hit )
    {
	send_to_char( "Such cowardice ill becomes you.\r\n", ch );
	return;
    }

    ch->wimpy	= wimpy;
    sprintf( buf, "Wimpy set to %d hit points.\r\n", wimpy );
    send_to_char( buf, ch );
    return;
}



void do_password( CHAR_DATA *ch, char *argument )
{
    char *pArg;
    char *pwdnew;
    char *p;
    char  arg1 [ MAX_INPUT_LENGTH ];
    char  arg2 [ MAX_INPUT_LENGTH ];
    char  cEnd;

    if ( IS_NPC( ch ) )
	return;

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
    while ( isspace( *argument ) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while ( isspace( *argument ) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';
    *argument = '\0';

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: password <old> <new>.\r\n", ch );
	return;
    }

    if ( strcmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) )
    {
	WAIT_STATE( ch, 40 );
	send_to_char( "Wrong password.  Wait 10 seconds.\r\n", ch );
	return;
    }

    if ( strlen( arg2 ) < 5 )
    {
	send_to_char(
	    "New password must be at least five characters long.\r\n", ch );
	return;
    }

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = crypt( arg2, ch->name );
    for ( p = pwdnew; *p != '\0'; p++ )
    {
	if ( *p == '~' )
	{
	    send_to_char(
		"New password not acceptable, try again.\r\n", ch );
	    return;
	}
    }

    free_string( ch->pcdata->pwd );
    ch->pcdata->pwd = str_dup( pwdnew );
    save_char_obj( ch );
    send_to_char( "Ok.\r\n", ch );
    return;
}



void do_socials( CHAR_DATA *ch, char *argument )
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  iSocial;
    int  col;

    buf1[0] = '\0';
    col = 0;
    for ( iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++ )
    {
	sprintf( buf, "%-12s", social_table[iSocial].name );
	strcat( buf1, buf );
	if ( ++col % 6 == 0 )
	    strcat( buf1, "\r\n" );
    }
 
    if ( col % 6 != 0 )
	strcat( buf1, "\r\n" );

    send_to_char( buf1, ch );
    return;
}



/*
 * Contributed by Alander.
 */
void do_commands( CHAR_DATA *ch, char *argument )
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  cmd;
    int  col;
 
    buf1[0] = '\0';
    col = 0;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if (   cmd_table[cmd].level <  LEVEL_HERO
	    && cmd_table[cmd].level <= get_trust( ch ) )
	{
	    sprintf( buf, "%-16s", cmd_table[cmd].name );
	    strcat( buf1, buf );
	    if ( ++col % 5 == 0 )
		strcat( buf1, "\r\n" );
	}
    }
 
    if ( col % 5 != 0 )
	strcat( buf1, "\r\n" );

    send_to_char( buf1, ch );
    return;
}



void do_channels( CHAR_DATA *ch, char *argument )
{
    char arg [ MAX_INPUT_LENGTH  ];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_SILENCE ) )
	{
	    send_to_char( "You are silenced.\r\n", ch );
	    return;
	}

	send_to_char( "Channels:", ch );

	send_to_char( !IS_SET( ch->deaf, CHANNEL_AUCTION  )
		     ? " +AUCTION"
		     : " -auction",
		     ch );

	send_to_char( !IS_SET( ch->deaf, CHANNEL_CHAT     )
		     ? " +CHAT"
		     : " -chat",
		     ch );

	if ( IS_HERO( ch ) )
	{
	    send_to_char( !IS_SET( ch->deaf, CHANNEL_IMMTALK )
			 ? " +IMMTALK"
			 : " -immtalk",
			 ch );
	}

	send_to_char( !IS_SET( ch->deaf, CHANNEL_MUSIC    )
		     ? " +MUSIC"
		     : " -music",
		     ch );

	send_to_char( !IS_SET( ch->deaf, CHANNEL_QUESTION )
		     ? " +QUESTION"
		     : " -question",
		     ch );

	send_to_char( !IS_SET( ch->deaf, CHANNEL_SHOUT    )
		     ? " +SHOUT"
		     : " -shout",
		     ch );

	send_to_char( !IS_SET( ch->deaf, CHANNEL_YELL     )
		     ? " +YELL"
		     : " -yell",
		     ch );

	send_to_char( ".\r\n", ch );
    }
    else
    {
	int  bit;
	bool fClear;

	     if ( arg[0] == '+' ) fClear = TRUE;
	else if ( arg[0] == '-' ) fClear = FALSE;
	else
	{
	    send_to_char( "Channels -channel or +channel?\r\n", ch );
	    return;
	}

	     if ( !str_cmp( arg+1, "auction"  ) ) bit = CHANNEL_AUCTION;
        else if ( !str_cmp( arg+1, "chat"     ) ) bit = CHANNEL_CHAT;
	else if ( !str_cmp( arg+1, "immtalk"  ) ) bit = CHANNEL_IMMTALK;
	else if ( !str_cmp( arg+1, "music"    ) ) bit = CHANNEL_MUSIC;
	else if ( !str_cmp( arg+1, "question" ) ) bit = CHANNEL_QUESTION;
	else if ( !str_cmp( arg+1, "shout"    ) ) bit = CHANNEL_SHOUT;
	else if ( !str_cmp( arg+1, "yell"     ) ) bit = CHANNEL_YELL;
	else
	{
	    send_to_char( "Set or clear which channel?\r\n", ch );
	    return;
	}

	if ( fClear )
	    REMOVE_BIT ( ch->deaf, bit );
	else
	    SET_BIT    ( ch->deaf, bit );

	send_to_char( "Ok.\r\n", ch );
    }

    return;
}



/*
 * Contributed by Grodyn.
 */
void do_config( CHAR_DATA *ch, char *argument )
{
    char arg [ MAX_INPUT_LENGTH ];

    if ( IS_NPC( ch ) )
	return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "[ Keyword  ] Option\r\n", ch );

	send_to_char(  IS_SET( ch->act, PLR_AUTOEXIT  )
            ? "[+AUTOEXIT ] You automatically see exits.\r\n"
	    : "[-autoexit ] You don't automatically see exits.\r\n"
	    , ch );

	send_to_char(  IS_SET( ch->act, PLR_AUTOLOOT  )
	    ? "[+AUTOLOOT ] You automatically loot corpses.\r\n"
	    : "[-autoloot ] You don't automatically loot corpses.\r\n"
	    , ch );

	send_to_char(  IS_SET( ch->act, PLR_AUTOGOLD  )
	    ? "[+AUTOGOLD ] You automatically get gold from corpses.\r\n"
	    : "[-autogold ] You don't automatically get gold from corpses.\r\n"
	    , ch );

	send_to_char(  IS_SET( ch->act, PLR_AUTOSAC   )
	    ? "[+AUTOSAC  ] You automatically sacrifice corpses.\r\n"
	    : "[-autosac  ] You don't automatically sacrifice corpses.\r\n"
	    , ch );

	send_to_char(  IS_SET( ch->act, PLR_BLANK     )
	    ? "[+BLANK    ] You have a blank line before your prompt.\r\n"
	    : "[-blank    ] You have no blank line before your prompt.\r\n"
	    , ch );

	send_to_char(  IS_SET( ch->act, PLR_BRIEF     )
	    ? "[+BRIEF    ] You see brief descriptions.\r\n"
	    : "[-brief    ] You see long descriptions.\r\n"
	    , ch );
         
	send_to_char(  IS_SET( ch->act, PLR_COMBINE   )
	    ? "[+COMBINE  ] You see object lists in combined format.\r\n"
	    : "[-combine  ] You see object lists in single format.\r\n"
	    , ch );

	send_to_char(  IS_SET( ch->act, PLR_PROMPT    )
	    ? "[+PROMPT   ] You have a prompt.\r\n"
	    : "[-prompt   ] You don't have a prompt.\r\n"
	    , ch );

	send_to_char(  IS_SET( ch->act, PLR_TELNET_GA )
	    ? "[+TELNETGA ] You receive a telnet GA sequence.\r\n"
	    : "[-telnetga ] You don't receive a telnet GA sequence.\r\n"
	    , ch );

	send_to_char(  IS_SET( ch->act, PLR_SILENCE   )
	    ? "[+SILENCE  ] You are silenced.\r\n"
	    : ""
	    , ch );

	send_to_char( !IS_SET( ch->act, PLR_NO_EMOTE  )
	    ? ""
	    : "[-emote    ] You can't emote.\r\n"
	    , ch );

	send_to_char( !IS_SET( ch->act, PLR_NO_TELL   )
	    ? ""
	    : "[-tell     ] You can't use 'tell'.\r\n"
	    , ch );
    }
    else
    {
	char buf [ MAX_STRING_LENGTH ];
	int  bit;
	bool fSet;

	     if ( arg[0] == '+' ) fSet = TRUE;
	else if ( arg[0] == '-' ) fSet = FALSE;
	else
	{
	    send_to_char( "Config -option or +option?\r\n", ch );
	    return;
	}

             if ( !str_cmp( arg+1, "autoexit" ) ) bit = PLR_AUTOEXIT;
	else if ( !str_cmp( arg+1, "autoloot" ) ) bit = PLR_AUTOLOOT;
	else if ( !str_cmp( arg+1, "autogold" ) ) bit = PLR_AUTOGOLD;
	else if ( !str_cmp( arg+1, "autosac"  ) ) bit = PLR_AUTOSAC;
	else if ( !str_cmp( arg+1, "blank"    ) ) bit = PLR_BLANK;
	else if ( !str_cmp( arg+1, "brief"    ) ) bit = PLR_BRIEF;
	else if ( !str_cmp( arg+1, "combine"  ) ) bit = PLR_COMBINE;
        else if ( !str_cmp( arg+1, "prompt"   ) ) bit = PLR_PROMPT;
	else if ( !str_cmp( arg+1, "telnetga" ) ) bit = PLR_TELNET_GA;
	else
	{
	    send_to_char( "Config which option?\r\n", ch );
	    return;
	}

	if ( fSet )
	{
	    SET_BIT    ( ch->act, bit );
	    sprintf( buf, "%s is now ON.\r\n", arg+1 );
	    buf[0] = UPPER( buf[0] );
	    send_to_char( buf, ch );
	}
	else
	{
	    REMOVE_BIT ( ch->act, bit );
	    sprintf( buf, "%s is now OFF.\r\n", arg+1 );
	    buf[0] = UPPER( buf[0] );
	    send_to_char( buf, ch );
	}

    }

    return;
}

void do_wizlist ( CHAR_DATA *ch, char *argument )
{

    do_help ( ch, "wizlist" );
    return;

}

void do_spells ( CHAR_DATA *ch, char *argument )
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  sn;
    int  col;

    if ( IS_NPC( ch )
	|| ( !IS_NPC( ch ) && !class_table[ch->class].fMana ) )
    {  
       send_to_char ( "You don't need no stinking spells!\r\n", ch );
       return;
    }

    buf1[0] = '\0';

    col = 0;
    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
        if ( !skill_table[sn].name )
	   break;
	if ( ( ch->level < skill_table[sn].skill_level[ch->class] )
	    || ( skill_table[sn].skill_level[ch->class] > LEVEL_HERO ) )
	   continue;

	sprintf ( buf, "%18s %3dpts ",
           skill_table[sn].name, MANA_COST( ch, sn ) );
	strcat( buf1, buf );
	if ( ++col % 3 == 0 )
	   strcat( buf1, "\r\n" );
    }

    if ( col % 3 != 0 )
      strcat( buf1, "\r\n" );

    send_to_char ( buf1, ch );
    return;

}

void do_slist ( CHAR_DATA *ch, char *argument )
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  sn;
    int  col;
    int  level;
    bool pSpell;

    if ( IS_NPC( ch ) )
    {  
       send_to_char ( "You do not need any stinking spells!\r\n", ch );
       return;
    }

    buf1[0] = '\0';

    strcat ( buf1, "ALL Abilities available for your class.\r\n\r\n" );
    strcat ( buf1, "Lv          Abilities\r\n\r\n" );

    for ( level = 1; level <= LEVEL_HERO; level++ )
    {

      col = 0;
      pSpell = TRUE;

      for ( sn = 0; sn < MAX_SKILL; sn++ )
      {
	if ( !skill_table[sn].name )
	  break;
	if ( skill_table[sn].skill_level[ch->class] != level )
	  continue;

	if ( pSpell )
	{
	  sprintf ( buf, "%2d:", level );
	  strcat ( buf1, buf );
	  pSpell = FALSE;
	}

	/* format fix by Koala */ 
	if ( ++col % 4 == 0 )
	  strcat ( buf1, "   " );

	sprintf ( buf, "%18s", skill_table[sn].name );
	strcat ( buf1, buf );

	if ( col % 4 == 0 )
	  strcat ( buf1, "\r\n" );

      }

      if ( col % 4 != 0 )
	strcat ( buf1, "\r\n" );

    }

    send_to_char( buf1, ch );
    return;

}

/* bypassing the config command - Kahn */

void do_autoexit ( CHAR_DATA *ch, char *argument )
{
    char buf[ MAX_STRING_LENGTH ];

    ( IS_SET ( ch->act, PLR_AUTOEXIT )
     ? sprintf( buf, "-autoexit" )
     : sprintf( buf, "+autoexit" ) );

    do_config( ch, buf );

    return;

}

void do_autoloot ( CHAR_DATA *ch, char *argument )
{
    char buf[ MAX_STRING_LENGTH ];

    ( IS_SET ( ch->act, PLR_AUTOLOOT )
     ? sprintf( buf, "-autoloot" )
     : sprintf( buf, "+autoloot" ) );

    do_config( ch, buf );

    return;
}

void do_autogold ( CHAR_DATA *ch, char *argument )
{
    char buf[ MAX_STRING_LENGTH ];

    ( IS_SET ( ch->act, PLR_AUTOGOLD )
     ? sprintf( buf, "-autogold" )
     : sprintf( buf, "+autogold" ) );

    do_config( ch, buf );

    return;

}

void do_autosac ( CHAR_DATA *ch, char *argument )
{
    char buf[ MAX_STRING_LENGTH ];

    ( IS_SET ( ch->act, PLR_AUTOSAC )
     ? sprintf( buf, "-autosac" )
     : sprintf( buf, "+autosac" ) );

    do_config( ch, buf );

    return;

}

void do_blank ( CHAR_DATA *ch, char *argument )
{
    char buf[ MAX_STRING_LENGTH ];

    ( IS_SET ( ch->act, PLR_BLANK )
     ? sprintf( buf, "-blank" )
     : sprintf( buf, "+blank" ) );

    do_config( ch, buf );

    return;

}

void do_brief ( CHAR_DATA *ch, char *argument )
{
    char buf[ MAX_STRING_LENGTH ];

    ( IS_SET ( ch->act, PLR_BRIEF )
     ? sprintf( buf, "-brief" )
     : sprintf( buf, "+brief" ) ) ; 

    do_config( ch, buf );

    return;

}

void do_combine ( CHAR_DATA *ch, char *argument )
{
    char buf[ MAX_STRING_LENGTH ];

    ( IS_SET ( ch->act, PLR_COMBINE )
     ? sprintf( buf, "-combine" )
     : sprintf( buf, "+combine" ) );

    do_config( ch, buf );

    return;

}
 
void do_pagelen ( CHAR_DATA *ch, char *argument )
{
    char buf [ MAX_STRING_LENGTH ];
    char arg [ MAX_INPUT_LENGTH  ];
    int  lines;

    if ( IS_NPC( ch ) )
        return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
	lines = 20;
    else
	lines = atoi( arg );

    if ( lines < 19 )
    {
	send_to_char( "Less than 19 lines of paged text is not allowed", ch );
	return;
    }

    if ( lines > 60 )
    {
	send_to_char(
		"I don't know of a screen that is larger than 60 lines!\r\n",
		     ch );
	lines = 60;
    }

    ch->pcdata->pagelen = lines;
    sprintf( buf, "Page pause set to %d lines.\r\n", lines );
    send_to_char( buf, ch );
    return;
}

/* Do_prompt from Morgenes from Aldara Mud */
void do_prompt( CHAR_DATA *ch, char *argument )
{
   char buf [ MAX_STRING_LENGTH ];

   buf[0] = '\0';

   /* Unswitched NPC's get kicked out */
   if ( !ch->desc )
       return;

   /* Will always have a pc ch after this */
   ch = ( ch->desc->original ? ch->desc->original : ch->desc->character );

   if ( argument[0] == '\0' )
   {
       ( IS_SET ( ch->act, PLR_PROMPT )
	? sprintf( buf, "-prompt" )
	: sprintf( buf, "+prompt" ) );

       do_config( ch, buf );

       return;
   }

   if( !strcmp( argument, "all" ) )
      strcat( buf, "<%hhp %mm %vmv> ");
   else
   {
      if ( strlen( argument ) > 50 )
	  argument[50] = '\0';
      smash_tilde( argument );
      strcat( buf, argument );
   }

   free_string( ch->pcdata->prompt );
   ch->pcdata->prompt = str_dup( buf );
   send_to_char( "Ok.\r\n", ch );
   return;
} 

void do_auto( CHAR_DATA *ch, char *argument )
{

    do_config( ch, "" );
    return;

}

void do_afk( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch ) )
        return;

    if ( IS_SET( ch->act, PLR_AFK ) )
    {
	REMOVE_BIT( ch->act, PLR_AFK );
	send_to_char( "You are back at your keyboard.\r\n", ch        );
	act( "$n has returned to $s keyboard.", ch, NULL, ch, TO_ROOM );
    }
    else
    {
	SET_BIT( ch->act, PLR_AFK );
	send_to_char( "You are now away from keyboard.\r\n", ch       );
	act( "$n has left $s keyboard.", ch, NULL, ch, TO_ROOM        );
    }
    
    return;
}
	
