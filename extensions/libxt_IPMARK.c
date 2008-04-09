/* Shared library add-on to iptables to add IPMARK target support.
 * (C) 2003 by Grzegorz Janoszka <Grzegorz.Janoszka@pro.onet.pl>
 *
 * based on original MARK target
 *
 * This program is distributed under the terms of GNU GPL
 */
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xtables.h>
#include "xt_IPMARK.h"

enum {
	FL_ADDR_USED     = 1 << 0,
	FL_AND_MASK_USED = 1 << 1,
	FL_OR_MASK_USED  = 1 << 2,
	FL_SHIFT         = 1 << 3,
};

/* Function which prints out usage message. */
static void ipmark_tg_help(void)
{
	printf(
"IPMARK target options:\n"
"  --addr {src|dst}    use source or destination ip address\n"
"  --and-mask value    logical AND ip address with this value becomes MARK\n"
"  --or-mask value     logical OR ip address with this value becomes MARK\n"
"  --shift value       shift address right by value before copying to mark\n"
"\n");
}

static const struct option ipmark_tg_opts[] = {
	{.name = "addr",     .has_arg = true, .val = '1'},
	{.name = "and-mask", .has_arg = true, .val = '2'},
	{.name = "or-mask",  .has_arg = true, .val = '3'},
	{.name = "shift",    .has_arg = true, .val = '4'},
	{NULL},
};

/* Initialize the target. */
static void ipmark_tg_init(struct xt_entry_target *t)
{
	struct xt_ipmark_tginfo *info = (void *)t->data;

	info->andmask = ~0U;
}

static int ipmark_tg_parse(int c, char **argv, int invert, unsigned int *flags,
                           const void *entry, struct xt_entry_target **target)
{
	struct xt_ipmark_tginfo *info = (void *)(*target)->data;
	unsigned int n;

	switch (c) {
	case '1':
		param_act(P_ONLY_ONCE, "IPMARK", "addr", *flags & FL_ADDR_USED);
		param_act(P_NO_INVERT, "IPMARK", "addr", invert);
		if (strcmp(optarg, "src") == 0)
			info->selector = XT_IPMARK_SRC;
		else if (strcmp(optarg, "dst") == 0)
			info->selector = XT_IPMARK_DST;
		else
			exit_error(PARAMETER_PROBLEM, "Bad addr value `%s' - should be `src' or `dst'", optarg);
		*flags |= FL_ADDR_USED;
		return true;
	
	case '2':
		param_act(P_ONLY_ONCE, "IPMARK", "and-mask", *flags & FL_AND_MASK_USED);
		param_act(P_NO_INVERT, "IPMARK", "and-mask", invert);
		if (!strtonum(optarg, NULL, &n, 0, ~0U))
			param_act(P_BAD_VALUE, "IPMARK", "and-mask", optarg);
		info->andmask = n;
		*flags |= FL_AND_MASK_USED;
		return true;

	case '3':
		param_act(P_ONLY_ONCE, "IPMARK", "or-mask", *flags & FL_OR_MASK_USED);
		param_act(P_NO_INVERT, "IPMARK", "or-mask", invert);
		if (!strtonum(optarg, NULL, &n, 0, ~0U))
			param_act(P_BAD_VALUE, "IPMARK", "or-mask", optarg);
		info->ormask = n;
		*flags |= FL_OR_MASK_USED;
		return true;

	case '4':
		param_act(P_ONLY_ONCE, "IPMARK", "--shift", *flags & FL_SHIFT);
		param_act(P_NO_INVERT, "IPMARK", "--shift", invert);
		/*
		 * Anything >31 does not make sense for IPv4, but it still
		 * does the right thing.
		 */
		if (!strtonum(optarg, NULL, &n, 0, 128))
			param_act(P_BAD_VALUE, "IPMARK", "--shift", optarg);
		info->shift = n;
		return true;
	}

	return false;
}

static void ipmark_tg_check(unsigned int flags)
{
	if (!(flags & FL_ADDR_USED))
		exit_error(PARAMETER_PROBLEM,
		           "IPMARK target: Parameter --addr is required");
}

static void
ipmark_tg_print(const void *entry, const struct xt_entry_target *target,
                int numeric)
{
	const struct xt_ipmark_tginfo *info = (const void *)target->data;

	if (info->selector == XT_IPMARK_SRC)
		printf("IPMARK src ip");
	else
		printf("IPMARK dst ip");

	if (info->andmask != ~0U)
		printf(" and 0x%x ", (unsigned int)info->andmask);
	if (info->ormask != 0)
		printf(" or 0x%x ", (unsigned int)info->ormask);
}

static void
ipmark_tg_save(const void *entry, const struct xt_entry_target *target)
{
	const struct xt_ipmark_tginfo *info = (const void *)target->data;

	if (info->selector == XT_IPMARK_SRC)
		printf("--addr src ");
	else
		printf("--addr dst ");

	if (info->andmask != ~0U)
		printf("--and-mask 0x%x ", (unsigned int)info->andmask);
	if (info->ormask != 0)
		printf("--or-mask 0x%x ", (unsigned int)info->ormask);
}

static struct xtables_target ipmark_tg4_reg = {
	.version       = XTABLES_VERSION,
	.name          = "IPMARK",
	.family        = PF_INET,
	.revision      = 0,
	.size          = XT_ALIGN(sizeof(struct xt_ipmark_tginfo)),
	.userspacesize = XT_ALIGN(sizeof(struct xt_ipmark_tginfo)),
	.help          = ipmark_tg_help,
	.init          = ipmark_tg_init,
	.parse         = ipmark_tg_parse,
	.final_check   = ipmark_tg_check,
	.print         = ipmark_tg_print,
	.save          = ipmark_tg_save,
	.extra_opts    = ipmark_tg_opts,
};

static struct xtables_target ipmark_tg6_reg = {
	.version       = XTABLES_VERSION,
	.name          = "IPMARK",
	.family        = PF_INET6,
	.revision      = 0,
	.size          = XT_ALIGN(sizeof(struct xt_ipmark_tginfo)),
	.userspacesize = XT_ALIGN(sizeof(struct xt_ipmark_tginfo)),
	.help          = ipmark_tg_help,
	.init          = ipmark_tg_init,
	.parse         = ipmark_tg_parse,
	.final_check   = ipmark_tg_check,
	.print         = ipmark_tg_print,
	.save          = ipmark_tg_save,
	.extra_opts    = ipmark_tg_opts,
};

static void _init(void)
{
	xtables_register_target(&ipmark_tg4_reg);
	xtables_register_target(&ipmark_tg6_reg);
}