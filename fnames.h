/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Fnames.h - Names of data files for Exult.
 **
 **	Written: 6/4/99 - JSF
 **/

#ifndef INCL_FNAMES
#define INCL_FNAMES	1

// This will get prepended with different things at runtime
// depending on the OS
#define	USER_CONFIGURATION_FILE	"exult.cfg"

/*
 *	Here are the files we use:
 */
#define GAMEDAT		"<GAMEDAT>/"
#define SHAPES_VGA	"<STATIC>/shapes.vga"
#define FACES_VGA	"<STATIC>/faces.vga"
#define GUMPS_VGA	"<STATIC>/gumps.vga"
#define FONTS_VGA	"<STATIC>/fonts.vga"
#define SPRITES_VGA     "<STATIC>/sprites.vga"
#define MAINSHP_FLX     "<STATIC>/mainshp.flx"
#define ENDSHAPE_FLX    "<STATIC>/endshape.flx"
#define SHPDIMS		"<STATIC>/shpdims.dat"
#define TFA		"<STATIC>/tfa.dat"
#define WGTVOL		"<STATIC>/wgtvol.dat"		
#define U7CHUNKS	"<STATIC>/u7chunks"
#define U7MAP		"<STATIC>/u7map"
#define TEXT_FLX	"<STATIC>/text.flx"
#define U7IFIX		"<STATIC>/u7ifix"
#define U7IREG		"<GAMEDAT>/u7ireg"
#define PALETTES_FLX	"<STATIC>/palettes.flx"
#define U7NBUF_DAT	"<GAMEDAT>/u7nbuf.dat"
#define NPC_DAT		"<GAMEDAT>/npc.dat"
#define MONSNPCS	"<GAMEDAT>/monsnpcs.dat"
#define USEDAT		"<GAMEDAT>/usecode.dat"
#define FLAGINIT	"<GAMEDAT>/flaginit"
#define GWINDAT		"<GAMEDAT>/gamewin.dat"
#define SCHEDULE_DAT	"<STATIC>/schedule.dat"
#define SHPDIMS_DAT	"<STATIC>/shpdims.dat"
#define INITGAME	"<STATIC>/initgame.dat"
#define USECODE		"<STATIC>/usecode"
#define POINTERS	"<STATIC>/pointers.shp"
#define MAINMUS		"<STATIC>/mt32mus.dat"
#define INTROMUS	"<STATIC>/intrordm.dat"
#define	XMIDI_MT	"<STATIC>/xmidi.mt"
#define U7SPEECH	"<STATIC>/u7speech.spc"
#define XFORMTBL       	"<STATIC>/xform.tbl"
#define MONSTERS	"<STATIC>/monsters.dat"
#define EQUIP		"<STATIC>/equip.dat"
#define READY		"<STATIC>/ready.dat"
#define WIHH		"<STATIC>/wihh.dat"
#define IDENTITY	"<GAMEDAT>/identity"
#define ENDGAME		"<STATIC>/endgame.dat"
#define ENDSCORE_XMI	"<STATIC>/endscore.xmi"
#define MIDITMPFILE     "u7midi"
#define SAVENAME	"game%02d.u7"
#define INTROSND	"<STATIC>/introsnd.dat"
#define ARMOR		"<STATIC>/armor.dat"
#define WEAPONS		"<STATIC>/weapons.dat"
#define PAPERDOL	"<STATIC>/paperdol.vga"

#define EXTRA_FONTS     32
#define ENDGAME_FONT1	(EXTRA_FONTS)
#define ENDGAME_FONT2	(EXTRA_FONTS+1)
#define ENDGAME_FONT3	(EXTRA_FONTS+2)
#define ENDGAME_FONT4	(EXTRA_FONTS+3)
#define MAINSHP_FONT1	(EXTRA_FONTS+4)
#define SIINTRO_FONT1	(EXTRA_FONTS+5)
#define NUM_FONTS	(SIINTRO_FONT1+1)

#endif

