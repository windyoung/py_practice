#ifndef _ZX_DBCMYSQLTYPE_HXX_
#define _ZX_DBCMYSQLTYPE_HXX_
    struct st_vio;    /* Only C */
    typedef struct st_vio Vio;

    #ifndef my_socket_defined
        #ifdef __WIN__
            #define my_socket SOCKET
        #else
            typedef int my_socket;
        #endif /* __WIN__ */
    #endif /* my_socket_defined */

    typedef char my_bool;

    #define MYSQL_ERRMSG_SIZE    512
    #define SQLSTATE_LENGTH 5

    typedef struct st_net {
    #if !defined(CHECK_EMBEDDED_DIFFERENCES) || !defined(EMBEDDED_LIBRARY)
      Vio *vio;
      unsigned char *buff,*buff_end,*write_pos,*read_pos;
      my_socket fd;                    /* For Perl DBI/dbd */
      /*
        The following variable is set if we are doing several queries in one
        command ( as in LOAD TABLE ... FROM MASTER ),
        and do not want to confuse the client with OK at the wrong time
      */
      unsigned long remain_in_buf,length, buf_length, where_b;
      unsigned long max_packet,max_packet_size;
      unsigned int pkt_nr,compress_pkt_nr;
      unsigned int write_timeout, read_timeout, retry_count;
      int fcntl;
      unsigned int *return_status;
      unsigned char reading_or_writing;
      char save_char;
      my_bool unused0; /* Please remove with the next incompatible ABI change. */
      my_bool unused; /* Please remove with the next incompatible ABI change */
      my_bool compress;
      my_bool unused1; /* Please remove with the next incompatible ABI change. */
      /*
        Pointer to query object in query cache, do not equal NULL (0) for
        queries in cache that have not stored its results yet
      */
    #endif
      /*
        'query_cache_query' should be accessed only via query cache
        functions and methods to maintain proper locking.
      */
      unsigned char *query_cache_query;
      unsigned int last_errno;
      unsigned char error; 
      my_bool unused2; /* Please remove with the next incompatible ABI change. */
      my_bool return_errno;
      /** Client library error message buffer. Actually belongs to struct MYSQL. */
      char last_error[MYSQL_ERRMSG_SIZE];
      /** Client library sqlstate buffer. Set along with the error message. */
      char sqlstate[SQLSTATE_LENGTH+1];
      void *extension;
    #if defined(MYSQL_SERVER) && !defined(EMBEDDED_LIBRARY)
      /*
        Controls whether a big packet should be skipped.

        Initially set to FALSE by default. Unauthenticated sessions must have
        this set to FALSE so that the server can't be tricked to read packets
        indefinitely.
      */
      my_bool skip_big_packet;
    #endif
    } NET;

    enum enum_field_types { MYSQL_TYPE_DECIMAL, MYSQL_TYPE_TINY,
                MYSQL_TYPE_SHORT,  MYSQL_TYPE_LONG,
                MYSQL_TYPE_FLOAT,  MYSQL_TYPE_DOUBLE,
                MYSQL_TYPE_NULL,   MYSQL_TYPE_TIMESTAMP,
                MYSQL_TYPE_LONGLONG,MYSQL_TYPE_INT24,
                MYSQL_TYPE_DATE,   MYSQL_TYPE_TIME,
                MYSQL_TYPE_DATETIME, MYSQL_TYPE_YEAR,
                MYSQL_TYPE_NEWDATE, MYSQL_TYPE_VARCHAR,
                MYSQL_TYPE_BIT,
                            MYSQL_TYPE_NEWDECIMAL=246,
                MYSQL_TYPE_ENUM=247,
                MYSQL_TYPE_SET=248,
                MYSQL_TYPE_TINY_BLOB=249,
                MYSQL_TYPE_MEDIUM_BLOB=250,
                MYSQL_TYPE_LONG_BLOB=251,
                MYSQL_TYPE_BLOB=252,
                MYSQL_TYPE_VAR_STRING=253,
                MYSQL_TYPE_STRING=254,
                MYSQL_TYPE_GEOMETRY=255

    };


    typedef struct st_mysql_field {
      char *name;                 /* Name of column */
      char *org_name;             /* Original column name, if an alias */
      char *table;                /* Table of column if column was a field */
      char *org_table;            /* Org table name, if table was an alias */
      char *db;                   /* Database for table */
      char *catalog;          /* Catalog for table */
      char *def;                  /* Default value (set by mysql_list_fields) */
      unsigned long length;       /* Width of column (create length) */
      unsigned long max_length;   /* Max width for selected set */
      unsigned int name_length;
      unsigned int org_name_length;
      unsigned int table_length;
      unsigned int org_table_length;
      unsigned int db_length;
      unsigned int catalog_length;
      unsigned int def_length;
      unsigned int flags;         /* Div flags */
      unsigned int decimals;      /* Number of decimals in field */
      unsigned int charsetnr;     /* Character set */
      enum enum_field_types type; /* Type of field. See mysql_com.h for types */
      void *extension;
    } MYSQL_FIELD;

    typedef struct st_used_mem
    {                   /* struct for once_alloc (block) */
      struct st_used_mem *next;       /* Next block in use */
      unsigned int    left;           /* memory left in block  */
      unsigned int    size;           /* size of block */
    } USED_MEM;


    typedef struct st_mem_root
    {
      USED_MEM *free;                  /* blocks with free memory in it */
      USED_MEM *used;                  /* blocks almost without free memory */
      USED_MEM *pre_alloc;             /* preallocated block */
                                       /* if block have less memory it will be put in 'used' list */
      size_t min_malloc;
      size_t block_size;               /* initial block size */
      unsigned int block_num;          /* allocated blocks counter */
      /* 
         first free block in queue test counter (if it exceed 
         MAX_BLOCK_USAGE_BEFORE_DROP block will be dropped in 'used' list)
      */
      unsigned int first_block_usage;

      void (*error_handler)(void);
    } MEM_ROOT;

    #ifndef _global_h
        #if defined(NO_CLIENT_LONG_LONG)
            typedef unsigned long my_ulonglong;
        #elif defined (__WIN__)
            typedef unsigned __int64 my_ulonglong;
        #else
            typedef long long my_ulonglong;
        #endif
    #endif


    enum mysql_option 
    {
      MYSQL_OPT_CONNECT_TIMEOUT, MYSQL_OPT_COMPRESS, MYSQL_OPT_NAMED_PIPE,
      MYSQL_INIT_COMMAND, MYSQL_READ_DEFAULT_FILE, MYSQL_READ_DEFAULT_GROUP,
      MYSQL_SET_CHARSET_DIR, MYSQL_SET_CHARSET_NAME, MYSQL_OPT_LOCAL_INFILE,
      MYSQL_OPT_PROTOCOL, MYSQL_SHARED_MEMORY_BASE_NAME, MYSQL_OPT_READ_TIMEOUT,
      MYSQL_OPT_WRITE_TIMEOUT, MYSQL_OPT_USE_RESULT,
      MYSQL_OPT_USE_REMOTE_CONNECTION, MYSQL_OPT_USE_EMBEDDED_CONNECTION,
      MYSQL_OPT_GUESS_CONNECTION, MYSQL_SET_CLIENT_IP, MYSQL_SECURE_AUTH,
      MYSQL_REPORT_DATA_TRUNCATION, MYSQL_OPT_RECONNECT,
      MYSQL_OPT_SSL_VERIFY_SERVER_CERT
    };

    struct st_mysql_options {
      unsigned int connect_timeout, read_timeout, write_timeout;
      unsigned int port, protocol;
      unsigned long client_flag;
      char *host,*user,*password,*unix_socket,*db;
      struct st_dynamic_array *init_commands;
      char *my_cnf_file,*my_cnf_group, *charset_dir, *charset_name;
      char *ssl_key;                /* PEM key file */
      char *ssl_cert;                /* PEM cert file */
      char *ssl_ca;                    /* PEM CA file */
      char *ssl_capath;                /* PEM directory of CA-s? */
      char *ssl_cipher;                /* cipher to use */
      char *shared_memory_base_name;
      unsigned long max_allowed_packet;
      my_bool use_ssl;                /* if to use SSL or not */
      my_bool compress,named_pipe;
     /*
       On connect, find out the replication role of the server, and
       establish connections to all the peers
     */
      my_bool rpl_probe;
     /*
       Each call to mysql_real_query() will parse it to tell if it is a read
       or a write, and direct it to the slave or the master
     */
      my_bool rpl_parse;
     /*
       If set, never read from a master, only from slave, when doing
       a read that is replication-aware
     */
      my_bool no_master_reads;
    #if !defined(CHECK_EMBEDDED_DIFFERENCES) || defined(EMBEDDED_LIBRARY)
      my_bool separate_thread;
    #endif
      enum mysql_option methods_to_use;
      char *client_ip;
      /* Refuse client connecting to server if it uses old (pre-4.1.1) protocol */
      my_bool secure_auth;
      /* 0 - never report, 1 - always report (default) */
      my_bool report_data_truncation;

      /* function pointers for local infile support */
      int (*local_infile_init)(void **, const char *, void *);
      int (*local_infile_read)(void *, char *, unsigned int);
      void (*local_infile_end)(void *);
      int (*local_infile_error)(void *, char *, unsigned int);
      void *local_infile_userdata;
      void *extension;
    };

    enum mysql_status 
    {
      MYSQL_STATUS_READY, MYSQL_STATUS_GET_RESULT, MYSQL_STATUS_USE_RESULT,
      MYSQL_STATUS_STATEMENT_GET_RESULT
    };

    #define SCRAMBLE_LENGTH 20

    typedef struct st_list {
      struct st_list *prev,*next;
      void *data;
    } LIST;

    typedef struct st_mysql
    {
      NET        net;            /* Communication parameters */
      unsigned char    *connector_fd;        /* ConnectorFd for SSL */
      char        *host,*user,*passwd,*unix_socket,*server_version,*host_info;
      char          *info, *db;
      struct charset_info_st *charset;
      MYSQL_FIELD    *fields;
      MEM_ROOT    field_alloc;
      my_ulonglong affected_rows;
      my_ulonglong insert_id;        /* id if insert on table with NEXTNR */
      my_ulonglong extra_info;        /* Not used */
      unsigned long thread_id;        /* Id for connection in server */
      unsigned long packet_length;
      unsigned int    port;
      unsigned long client_flag,server_capabilities;
      unsigned int    protocol_version;
      unsigned int    field_count;
      unsigned int     server_status;
      unsigned int  server_language;
      unsigned int    warning_count;
      struct st_mysql_options options;
      enum mysql_status status;
      my_bool    free_me;        /* If free in mysql_close */
      my_bool    reconnect;        /* set to 1 if automatic reconnect */

      /* session-wide random string */
      char     scramble[SCRAMBLE_LENGTH+1];

     /*
       Set if this is the original connection, not a master or a slave we have
       added though mysql_rpl_probe() or mysql_set_master()/ mysql_add_slave()
     */
      my_bool rpl_pivot;
      /*
        Pointers to the master, and the next slave connections, points to
        itself if lone connection.
      */
      struct st_mysql* master, *next_slave;

      struct st_mysql* last_used_slave; /* needed for round-robin slave pick */
     /* needed for send/read/store/use result to work correctly with replication */
      struct st_mysql* last_used_con;

      LIST  *stmts;                     /* list of all statements */
      const struct st_mysql_methods *methods;
      void *thd;
      /*
        Points to boolean flag in MYSQL_RES  or MYSQL_STMT. We set this flag 
        from mysql_stmt_close if close had to cancel result set of this object.
      */
      my_bool *unbuffered_fetch_owner;
      /* needed for embedded server - no net buffer to store the 'info' */
      char *info_buffer;
      void *extension;
    } MYSQL;


    typedef struct st_mysql_bind
    {
      unsigned long    *length;          /* output length pointer */
      my_bool       *is_null;      /* Pointer to null indicator */
      void        *buffer;      /* buffer to get/put data */
      /* set this if you want to track data truncations happened during fetch */
      my_bool       *error;
      unsigned char *row_ptr;         /* for the current data position */
      void (*store_param_func)(NET *net, struct st_mysql_bind *param);
      void (*fetch_result)(struct st_mysql_bind *, MYSQL_FIELD *,
                           unsigned char **row);
      void (*skip_result)(struct st_mysql_bind *, MYSQL_FIELD *,
                  unsigned char **row);
      /* output buffer length, must be set when fetching str/binary */
      unsigned long buffer_length;
      unsigned long offset;           /* offset position for char/binary fetch */
      unsigned long    length_value;     /* Used if length is 0 */
      unsigned int    param_number;      /* For null count and error messages */
      unsigned int  pack_length;      /* Internal length for packed data */
      enum enum_field_types buffer_type;    /* buffer type */
      my_bool       error_value;      /* used if error is 0 */
      my_bool       is_unsigned;      /* set if integer type is unsigned */
      my_bool    long_data_used;      /* If used with mysql_send_long_data */
      my_bool    is_null_value;    /* Used if is_null is 0 */
      void *extension;
    } MYSQL_BIND;

    typedef char **MYSQL_ROW;        /* return data as array of strings */

    typedef struct st_mysql_rows {
      struct st_mysql_rows *next;        /* list of rows */
      MYSQL_ROW data;
      unsigned long length;
    } MYSQL_ROWS;

    typedef struct st_mysql_data {
      MYSQL_ROWS *data;
      struct embedded_query_result *embedded_info;
      MEM_ROOT alloc;
      my_ulonglong rows;
      unsigned int fields;
      /* extra info for embedded library */
      void *extension;
    } MYSQL_DATA;

    /* statement state */
    enum enum_mysql_stmt_state
    {
      MYSQL_STMT_INIT_DONE= 1, MYSQL_STMT_PREPARE_DONE, MYSQL_STMT_EXECUTE_DONE,
      MYSQL_STMT_FETCH_DONE
    };

    enum enum_stmt_attr_type
    {
      /*
        When doing mysql_stmt_store_result calculate max_length attribute
        of statement metadata. This is to be consistent with the old API, 
        where this was done automatically.
        In the new API we do that only by request because it slows down
        mysql_stmt_store_result sufficiently.
      */
      STMT_ATTR_UPDATE_MAX_LENGTH,
      /*
        unsigned long with combination of cursor flags (read only, for update,
        etc)
      */
      STMT_ATTR_CURSOR_TYPE,
      /*
        Amount of rows to retrieve from server per one fetch if using cursors.
        Accepts unsigned long attribute in the range 1 - ulong_max
      */
      STMT_ATTR_PREFETCH_ROWS
    };


    /* statement handler */
    typedef struct st_mysql_stmt
    {
      MEM_ROOT       mem_root;             /* root allocations */
      LIST           list;                 /* list to keep track of all stmts */
      MYSQL          *mysql;               /* connection handle */
      MYSQL_BIND     *params;              /* input parameters */
      MYSQL_BIND     *bind;                /* output parameters */
      MYSQL_FIELD    *fields;              /* result set metadata */
      MYSQL_DATA     result;               /* cached result set */
      MYSQL_ROWS     *data_cursor;         /* current row in cached result */
      /*
        mysql_stmt_fetch() calls this function to fetch one row (it's different
        for buffered, unbuffered and cursor fetch).
      */
      int            (*read_row_func)(struct st_mysql_stmt *stmt, 
                                      unsigned char **row);
      /* copy of mysql->affected_rows after statement execution */
      my_ulonglong   affected_rows;
      my_ulonglong   insert_id;            /* copy of mysql->insert_id */
      unsigned long     stmt_id;           /* Id for prepared statement */
      unsigned long  flags;                /* i.e. type of cursor to open */
      unsigned long  prefetch_rows;        /* number of rows per one COM_FETCH */
      /*
        Copied from mysql->server_status after execute/fetch to know
        server-side cursor status for this statement.
      */
      unsigned int   server_status;
      unsigned int     last_errno;           /* error code */
      unsigned int   param_count;          /* input parameter count */
      unsigned int   field_count;          /* number of columns in result set */
      enum enum_mysql_stmt_state state;    /* statement state */
      char         last_error[MYSQL_ERRMSG_SIZE]; /* error message */
      char         sqlstate[SQLSTATE_LENGTH+1];
      /* Types of input parameters should be sent to server */
      my_bool        send_types_to_server;
      my_bool        bind_param_done;      /* input buffers were supplied */
      unsigned char  bind_result_done;     /* output buffers were supplied */
      /* mysql_stmt_close() had to cancel this result */
      my_bool       unbuffered_fetch_cancelled;  
      /*
        Is set to true if we need to calculate field->max_length for 
        metadata fields when doing mysql_stmt_store_result.
      */
      my_bool       update_max_length;     
      void *extension;
    } MYSQL_STMT;

    typedef struct st_mysql_res {
      my_ulonglong  row_count;
      MYSQL_FIELD    *fields;
      MYSQL_DATA    *data;
      MYSQL_ROWS    *data_cursor;
      unsigned long *lengths;        /* column lengths of current row */
      MYSQL        *handle;        /* for unbuffered reads */
      const struct st_mysql_methods *methods;
      MYSQL_ROW    row;            /* If unbuffered read */
      MYSQL_ROW    current_row;        /* buffer to current row */
      MEM_ROOT    field_alloc;
      unsigned int    field_count, current_field;
      my_bool    eof;            /* Used by mysql_fetch_row */
      /* mysql_stmt_close() had to cancel this result */
      my_bool       unbuffered_fetch_cancelled;  
      void *extension;
    } MYSQL_RES;

    enum enum_mysql_timestamp_type
    {
      MYSQL_TIMESTAMP_NONE= -2, MYSQL_TIMESTAMP_ERROR= -1,
      MYSQL_TIMESTAMP_DATE= 0, MYSQL_TIMESTAMP_DATETIME= 1, MYSQL_TIMESTAMP_TIME= 2
    };


    typedef struct st_mysql_time
    {
      unsigned int  year, month, day, hour, minute, second;
      unsigned long second_part;
      my_bool       neg;
      enum enum_mysql_timestamp_type time_type;
    } MYSQL_TIME;

    /* status return codes */
    #define MYSQL_NO_DATA        100
    #define MYSQL_DATA_TRUNCATED 101  

    enum enum_cursor_type
    {
      CURSOR_TYPE_NO_CURSOR= 0,
      CURSOR_TYPE_READ_ONLY= 1,
      CURSOR_TYPE_FOR_UPDATE= 2,
      CURSOR_TYPE_SCROLLABLE= 4
    };
#endif

