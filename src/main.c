/* vim:set et sts=4: */
#include <config.h>
#include <unistd.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <ibus.h>
#include <locale.h>

#include <libintl.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

#include "engine.h"

static IBusBus *bus = NULL;
static IBusFactory *factory = NULL;

static void ibus_disconnected_cb(IBusBus *bus, gpointer user_data)
{
	gtk_main_quit();
}

char *tablefile=  TABLEFILE ;
char iconfile[4096] =
{ 0 };

static void init_inside()
{
	IBusComponent *component;

	ibus_init();

	bus = ibus_bus_new();
	g_signal_connect (bus, "disconnected", G_CALLBACK (ibus_disconnected_cb), NULL);

	factory = ibus_factory_new(ibus_bus_get_connection(bus));

	ibus_bus_request_name(bus, "org.freedesktop.IBus.handwrite", 0);

	component = ibus_component_new("org.freedesktop.IBus.handwrite",
			"handwrite", PACKAGE_VERSION, "GPL", MICROCAI_WITHEMAIL, PACKAGE_BUGREPORT,
			PKGDATADIR, GETTEXT_PACKAGE);

	ibus_bus_register_component(bus, component);

	ibus_factory_add_engine(factory, "handwrite", IBUS_TYPE_HANDWRITE_ENGINE);

	g_object_unref(component);
}

static void init_outside(const char * iconfile, const char *exefile)
{
	IBusComponent *component;
	IBusEngineDesc * desc;

	ibus_init();

	bus = ibus_bus_new();
	g_signal_connect (bus, "disconnected", G_CALLBACK (ibus_disconnected_cb), NULL);

	factory = ibus_factory_new(ibus_bus_get_connection(bus));

	ibus_bus_request_name(bus, "org.freedesktop.IBus.handwrite", 0);

	desc = ibus_engine_desc_new("handwrite", "handwrite",
			_("hand write recognizer"), "zh_CN", "GPL",
			MICROCAI_WITHEMAIL, iconfile, "us");

	component = ibus_component_new("org.freedesktop.IBus.handwrite",
			"handwrite", PACKAGE_VERSION, "GPL", MICROCAI_WITHEMAIL, PACKAGE_BUGREPORT,
			exefile, GETTEXT_PACKAGE);

	ibus_component_add_engine(component, desc);

	ibus_bus_register_component(bus, component);

	ibus_factory_add_engine(factory, "handwrite", IBUS_TYPE_HANDWRITE_ENGINE);

	g_object_unref(component);
}

int main(int argc, char* argv[])
{
	GError * err = NULL;

	gboolean have_ibus=FALSE;

	const gchar * icon_file = "icons/ibus-handwrite.svg";

	setlocale(LC_ALL, "");
	gtk_set_locale();
	textdomain(GETTEXT_PACKAGE);
#ifdef DEBUG
	bindtextdomain(GETTEXT_PACKAGE, "/tmp/usr/share/locale");
#endif

	GOptionEntry args[] =
	{
			{"ibus",'\0',0,G_OPTION_ARG_NONE,&have_ibus},
			{"icon",'\0',0,G_OPTION_ARG_STRING,&icon_file,_("the icon file"),N_("icon file")},
			{"table",'\0',0,G_OPTION_ARG_STRING,&tablefile,_("set table file path"),N_("tablefile")},
			{0}
	};

	if(G_UNLIKELY(!gtk_init_with_args(&argc, &argv,PACKAGE_STRING,args,NULL,&err)))
	{
		g_error("%s",err->message);
	}

	ibus_init();

	if (!have_ibus)
	{
		char exefile[4096] =
		{ 0 };

		init_outside(realpath(icon_file, iconfile), realpath(argv[0], exefile));
		printf(_("ibus-handwrite Version %s Start Up\n"), PACKAGE_VERSION);
	}
	else
	{
		init_inside();
	}
	gtk_main();
	return 0;
}
