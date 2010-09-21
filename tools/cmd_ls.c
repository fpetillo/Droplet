/*
 * Droplets, high performance cloud storage client library
 * Copyright (C) 2010 Scality http://github.com/scality/Droplets
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *  
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dplsh.h"

int cmd_ls(int argc, char **argv);

struct usage_def ls_usage[] =
  {
    {'l', 0u, NULL, "long display"},
    {'P', USAGE_PARAM, "prefix", "default unset"},
    {'D', USAGE_PARAM, "delimiter", "default unset"},
    {USAGE_NO_OPT, 0u, "path or bucket", "remote directory"},
    {0, 0u, NULL, NULL},
  };

struct cmd_def ls_cmd = {"ls", "list directory", ls_usage, cmd_ls};

int
cmd_ls(int argc,
       char **argv)
{
  char opt;
  int ret;
  int lflag = 0;
  char *prefix = NULL;
  char *delimiter = NULL;
  size_t total_size = 0;
  dpl_vec_t *objects = NULL;
  dpl_vec_t *common_prefixes = NULL;
  void *dir_hdl = NULL;
  dpl_dirent_t entry;
  char *path;

  var_set("status", "1", VAR_CMD_SET, NULL);

  optind = 0;

  while ((opt = getopt(argc, argv, usage_getoptstr(ls_usage))) != -1)
    switch (opt)
      {
      case 'l':
        lflag = 1;
        break ;
      case 'P':
        prefix = xstrdup(optarg);
        break ;
      case 'D':
        delimiter = xstrdup(optarg);
        break ;
      case '?':
      default:
        usage_help(&ls_cmd);
        return SHELL_CONT;
      }
  argc -= optind;
  argv += optind;

  if (0 == argc)
    path = ".";
  else if (1 == argc)
    path = argv[0];
  else
    {
      usage_help(&ls_cmd);
      return SHELL_CONT;
    }
  
  ret = dpl_opendir(ctx, path, &dir_hdl);
  if (DPL_SUCCESS != ret)
    {
      fprintf(stderr, "opendir failure %s (%d)\n", dpl_status_str(ret), ret);
      return SHELL_CONT;
    }
  
  while (1 != dpl_vdir_eof(dir_hdl))
    {
      ret = dpl_vdir_readdir(dir_hdl, &entry);
      if (0 != ret)
        {
          fprintf(stderr, "read dir failure %d\n", ret);
          goto end;
        }
      
      if (lflag)
        {
          struct tm *stm;
          
          stm = localtime(&entry.last_modified);
          printf("%12llu %04d-%02d-%02d %02d:%02d %s\n", (unsigned long long) entry.size, 1900 + stm->tm_year, 1 + stm->tm_mon, stm->tm_mday, stm->tm_hour, stm->tm_min, entry.name);
        }
      else
        {
          printf("%s\n", entry.name);
        }
      
      total_size += entry.size;
    }

  if (1 == lflag)
    {
      if (NULL != ctx->pricing)
        printf("Total %s Price %s\n", dpl_size_str(total_size), dpl_price_storage_str(ctx, total_size));
    }

  var_set("status", "0", VAR_CMD_SET, NULL);

 end:

  if (NULL != dir_hdl)
    dpl_vdir_closedir(dir_hdl);

  if (NULL != objects)
    dpl_vec_objects_free(objects);

  if (NULL != common_prefixes)
    dpl_vec_common_prefixes_free(common_prefixes);

  if (NULL != prefix)
    free(prefix);

  if (NULL != delimiter)
    free(delimiter);

  return SHELL_CONT;
}