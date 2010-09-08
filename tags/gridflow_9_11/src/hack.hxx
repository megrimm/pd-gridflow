#if 1 /* included from Pd 41, sorry */
static void canvas_reflecttitle(t_canvas *x)
{
    char namebuf[MAXPDSTRING];
    t_canvasenvironment *env = canvas_getenv(x);
    if (env->ce_argc)
    {
        int i;
        strcpy(namebuf, " (");
        for (i = 0; i < env->ce_argc; i++)
        {
            if (strlen(namebuf) > MAXPDSTRING/2 - 5)
                break;
            if (i != 0)
                strcat(namebuf, " ");
            atom_string(&env->ce_argv[i], namebuf + strlen(namebuf),
                MAXPDSTRING/2);
        }
        strcat(namebuf, ")");
    }
    else namebuf[0] = 0;
    sys_vgui("wm title .x%lx {%s%c%s - %s}\n",
        long(x), x->gl_name->s_name, (x->gl_dirty? '*' : ' '), namebuf,
            canvas_getdir(x)->s_name);
}
#else
extern "C" void canvas_reflecttitle (t_glist *);
#endif
