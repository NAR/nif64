#include "erl_nif.h"

ERL_NIF_TERM mk_atom(ErlNifEnv* env, const char* atom);
ERL_NIF_TERM mk_error(ErlNifEnv* env, const char* mesg);

ERL_NIF_TERM
mk_atom(ErlNifEnv* env, const char* atom)
{
    ERL_NIF_TERM ret;

    if(!enif_make_existing_atom(env, atom, &ret, ERL_NIF_LATIN1))
    {
        return enif_make_atom(env, atom);
    }

    return ret;
}

ERL_NIF_TERM
mk_error(ErlNifEnv* env, const char* mesg)
{
    return enif_make_tuple2(env, mk_atom(env, "error"), mk_atom(env, mesg));
}

static ERL_NIF_TERM
repeat(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    ErlNifEnv* msg_env;
    ErlNifPid pid;

    if(argc != 2)
    {
        return enif_make_badarg(env);
    }

    if(!enif_is_pid(env, argv[0]))
    {
        return mk_error(env, "not_a_pid");
    }

    if(!enif_get_local_pid(env, argv[0], &pid))
    {
        return mk_error(env, "not_a_local_pid");
    }

    msg_env = enif_alloc_env();
    if(msg_env == NULL)
    {
        return mk_error(env, "environ_alloc_error");
    }

    u_int64_t correlation_int;
    if (enif_get_uint64(env, argv[1], &correlation_int) <= 0) {
        return mk_error(env, "cannot_get_correlation_id");
    }

    ERL_NIF_TERM val = enif_make_int64(env, correlation_int);

    if(!enif_send(env, &pid, msg_env, val))
    {
        enif_free(msg_env);
        return mk_error(env, "error_sending_term");
    }

    enif_free_env(msg_env);
    return mk_atom(env, "ok");
}

static ErlNifFunc nif_funcs[] = {
    {"repeat", 2, repeat}
};

ERL_NIF_INIT(test_nif, nif_funcs, NULL, NULL, NULL, NULL);

