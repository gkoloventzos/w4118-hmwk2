#!/bin/bash

check="../utils/bin/checkpatch.pl --terse --no-tree --file"

$check kernel/arch/arm/kernel/calls.S
$check kernel/include/linux/prinfo.h
$check kernel/kernel/ptree.c
$check tests/list.c
$check tests/list.h
$check tests/prinfo.c
$check tests/prinfo.h
