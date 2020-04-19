#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char buff[65536*2];
static int  _line;
static int  _patch;
static int _match;
static int _match_sd2;
static int _nullref;

typedef struct {
    char    *token;
    char    *reftext;
    char    *id;
    char    *localize;
    char    *comment;
} KDB;

static KDB  database[65535];
static int  nn= 0;

static const char *cComment = "-----";

static int  _dup_check_database( char *token, char *reftext, char *id ) {
int j;
    for( j = 0 ; j < nn ; j++ ) {
        if( token && database[j].token && !strcmp( database[j].token, token )) {
            if( reftext && database[j].reftext && !strcmp( database[j].reftext, reftext )) {
                if( id && database[j].id && !strcmp( database[j].id, id )) {
                    return( 2 );
                } else {
                    return( 2 );
                }
            } else {
                if( reftext && database[j].reftext && strcmp( database[j].reftext, reftext )) {
                    fprintf( stderr,"Same TOKEN, mismatch REFTEXT: %s (%s)-(%s)\n",token, reftext, database[j].reftext);
                    database[j].comment[0]=
                    database[j].comment[1]=
                    database[j].comment[2]=
                    database[j].comment[3]=
                    database[j].comment[4]='@';
                    return( 1 );
                }
            }
        }
    }
return( 0 );
}

static int  _add_database( char *token, char *reftext, char *id, char *remark ) {
    database[nn].token    = ( token ? strdup( token ) : NULL );
    database[nn].reftext  = ( reftext ? strdup( reftext ) : NULL );
    database[nn].id       = ( id ? strdup( id ) : NULL );
    database[nn].localize = NULL;
    database[nn].comment  = strdup( remark );
    nn++;
    if( nn > 65535 ) {
        fprintf(stderr,"Database limit!\n");
        exit( -1 );
    }
}

static int  _base_database( char *id, char *reftext ) {
int     i;
    for( i = 0 ; i < nn ; i++ ) {
        if( !strcmp( database[i].id, id )) {
            if( database[i].reftext ) {
                if( strcmp( database[i].reftext, reftext )) {
                    fprintf( stderr, "Unmatch reftext( %s : %s )\n", database[i].reftext, reftext );
                }
                break;
            }
            database[i].reftext = strdup( reftext );
            database[i].comment[2] = 'b';
            break;
        }
    }
return( 0 );
}



static int  _patch_database( char *id, char *reftext, char *text ) {
int     i;
    for( i = 0 ; i < nn ; i++ ) {
        if( !strcmp( database[i].id, id )) {
#if 1
            if( database[i].reftext ) {
                if( !strcmp( database[i].reftext, text )) {
                    break;
//                    fprintf( stderr, "Unmatch reftext( %s : %s )\n", database[i].reftext, reftext );
                }
            }
#endif
            database[i].comment[3] = 'p';
            database[i].reftext = strdup( text );
            _patch++;
            break;
        }
    }
return( 0 );
}

static int  _search_database( char *token, char *reftext ) {
int     i;
    for( i = 0 ; i < nn ; i++ ) {
        if( !strcmp( database[i].token, token )) {
            database[i].comment[4] = 'j';
#if 1
            database[i].localize = strdup( reftext );
#endif
            _match++;
            break;
        }
    }
return( 0 );
}

static int  _search_SD2_database( char *token, char *reftext ) {
int     i;
    for( i = 0 ; i < nn ; i++ ) {
        if( !strcmp( database[i].token, token )) {
            database[i].comment[1] = 'J';
            database[i].localize = strdup( reftext );
                _match_sd2++;
            break;
        }
    }
return( 0 );
}

static char *_find_token( char **src ) {
char    *p = *src;
int     bString = 0;

    if( *p == '\r' || *p == '\0' ) {
        return( NULL );
    }
    if( *p == ';' ) {
        (*src)++;
        return( NULL );
    }
    for(  ; **src != '\0' && **src != '\n' ; (*src)++ ) {
        if( **src == '\"' ) bString = !bString;
        if( !bString && ( **src == ';' || **src == '\r' )) {
            **src = 0x00;
            (*src)++;
           return( p );
        }
    }
    fprintf( stderr, " ; Not Found @ %d", _line );
    exit( -1 );
}

static int  _parse_entries( char *src ) {
char    *s;
char    *project;
char    *dico;
char    *token;
char    *reftext;
char    *id;
int     bDup;

    project = s = _find_token( &src );
    dico = s =  _find_token( &src );
    token = s =  _find_token( &src );
    reftext = s =  _find_token( &src );
    id = s =  _find_token( &src );

//    printf(" [%10s] [%10s] [%10s] [%10s] [%10s] \n",  project, dico, token, reftext, id );
    bDup = _dup_check_database( token, reftext, id );
    if( bDup == 0 ) {
        _add_database( token, reftext, id, "-----" );
    }
    if( bDup == 1 ) {
        _add_database( token, reftext, id, "@@@@@" );
    }
    return( 0 );
fail:
//    printf(" [%10s] [%10s] [%10s] [%10s] [%10s] \n",  project, dico, token, reftext, id );
    fprintf( stderr, "Parse error(%s) @ %d\n", src, _line );
return( -1 );
}
static int  _base_entries( char *src ) {
char    *id;
char    *reftext;
char    *comment;
char    *context;
char    *constraint;
char    *donttranslate;

    if( *src == '\n' ) return( 0 );
    id = _find_token( &src );
    reftext = _find_token( &src );
    comment = _find_token( &src );
    context = _find_token( &src );
    constraint = _find_token( &src );
    donttranslate = _find_token( &src );
    if( donttranslate && strcmp( donttranslate, "\"DO NOT TRANSLATE\"" )) return( 0 );

        if( reftext ) {
            _base_database( id, reftext );
//        } else if( context ) {
//            _base_database( id, context );
        }
return( 0 );
}

static int  _patch_entries( char *src ) {
char    *id;
char    *lang;
char    *reftext;
char    *text;

    if( *src == '\n' ) return( 0 );
    id = _find_token( &src );
    lang = _find_token( &src );
//    if( strcmp( lang, "US" )) return( 0 );

    reftext = _find_token( &src );
    text = _find_token( &src );
    if( !strcmp( lang, "\"US\"" )) {
        if( reftext ) {
            _patch_database( id, reftext, text );
        }
    }
return( 0 );
}

static int  _match_entries( char *src ) {
char    *s;
char    *token;
char    *comment;
char    *translate;
char    *constraint;
char    *reftext;
char    *id;

    if( *src == '\n' ) return( 0 );
    token = s = _find_token( &src );
    comment = s = _find_token( &src );
    translate = s =  _find_token( &src );
    constraint = s =  _find_token( &src );
    reftext = s = _find_token( &src );
    id = s = _find_token( &src );
    if( reftext ) {
        _search_database( token, reftext );
    }
    return( 0 );
fail:
//    printf(" [%10s] [%10s] [%10s] [%10s] [%10s] \n",  project, dico, token, reftext, id );
//    fprintf( stderr, "Parse error(%s) @ %d\n", src, _line );
return( 0 );
}

static int  _match_sd2_entries( char *src ) {
char    *s;
char    *token;
char    *comment;
char    *modebit;
char    *constraint;
char    *translate;
char    *reftext;

    if( *src == '\n' ) return( 0 );
    token = s = _find_token( &src );
    comment = s = _find_token( &src );
    modebit = s =  _find_token( &src );
    constraint = s =  _find_token( &src );
    translate = s = _find_token( &src );
    reftext = s = _find_token( &src );
    if( translate && reftext && strcmp( translate, reftext ) && modebit[2]=='J') {
        _search_SD2_database( token, translate );
    }
    if( translate && reftext && modebit[2]!='J' ) {
        fprintf( stderr,"***[%s] (%s:%s)\n", token, translate, reftext );
    }
    return( 0 );
fail:
//    printf(" [%10s] [%10s] [%10s] [%10s] [%10s] \n",  project, dico, token, reftext, id );
//    fprintf( stderr, "Parse error(%s) @ %d\n", src, _line );
return( 0 );
}

/******************************************************************************
 *      Function    : enclosing_method
 *      Description : 関数の機能説明
 *      Parameter   : パラメータ名:パラメータの説明
 *      Return val  : リターン値:リターンの意味
 *      Notes       : 関数を使用する上での注意事項
 */
int main( int argc, char *argv[] ) {
int      ret;
FILE    *fp;
char    *s;
int     i, j;
int     tokenlen;
char    spacer[30];

    /* ---------------- */
    /* read Entries.csv */
    /* ---------------- */
    fprintf( stderr, "databases/Entries.csv\n" );
    if((fp = fopen( "databases/Entries.csv", "r" )) != NULL ) {
        _line = 0;
        while(( s = fgets( buff, sizeof(buff), fp)) != NULL) {
            _line++;
            ret = _parse_entries( s );
            if( ret != 0 ) {
                break;
            }
        }
        fclose( fp );
    } else {
        fprintf( stderr, "File open error\n" );
        exit( -1 );
    }
    if( ret != 0 ) exit( -1 );

    /* ------------------- */
    /* read RefStrings.csv */
    /* ------------------- */
    fprintf( stderr, "databases/RefStrings.csv\n" );
    if((fp = fopen( "databases/RefStrings.csv", "r" )) != NULL ) {
        _line = 0;
        while(( s = fgets( buff, sizeof(buff), fp)) != NULL) {
            _line++;
            ret = _base_entries( s );
            if( ret != 0 ) {
                break;
            }
        }
        fclose( fp );
    } else {
        fprintf( stderr, "File open error\n" );
        exit( -1 );
    }
    if( ret != 0 ) exit( -1 );

    /* --------------------- */
    /* read Translations.csv */
    /* --------------------- */
    fprintf( stderr, "databases/Translations.csv\n" );
    if((fp = fopen( "databases/Translations.csv", "r" )) != NULL ) {
        _line = 0;
        _patch = 0;
        while(( s = fgets( buff, sizeof(buff), fp)) != NULL) {
            _line++;
            ret = _patch_entries( s );
            if( ret != 0 ) {
                break;
            }
        }
        fclose( fp );
    } else {
        fprintf( stderr, "File open error\n" );
        exit( -1 );
    }
    if( ret != 0 ) exit( -1 );

    /* ---------------- */
    /* read MODDING.csv */
    /* ---------------- */
    fprintf( stderr, "databases/MODDING.csv\n" );
    if((fp = fopen( "databases/MODDING.csv", "r" )) != NULL ) {
        _line = 0;
        _match = _match_sd2 = 0;
        while(( s = fgets( buff, sizeof(buff), fp)) != NULL) {
            _line++;
            ret = _match_entries( s );
            if( ret != 0 ) {
                break;
            }
        }
        fclose( fp );
    } else {
        fprintf( stderr, "File open error\n" );
        exit( -1 );
    }
    if( ret != 0 ) exit( -1 );

#if 1
    /* -------------------- */
    /* read src/MODDING.csv */
    /* -------------------- */
    fprintf( stderr, "src/MODDING.csv" );
    if((fp = fopen( "src/MODDING.csv", "r" )) != NULL ) {
        _line = 0;
        _match = _match_sd2 = 0;
        while(( s = fgets( buff, sizeof(buff), fp)) != NULL) {
            _line++;
            ret = _match_sd2_entries( s );
            if( ret != 0 ) {
                break;
            }
        }
        fclose( fp );
    } else {
        fprintf( stderr, "File open error\n" );
        exit( -1 );
    }
    if( ret != 0 ) exit( -1 );
#endif
    /* ------------ */
    /* OUTPUT files */
    /* ------------ */
    _nullref = 0;
    printf( "\"TOKEN\";\"COMMENT\";\"DO NOT TRANSLATE\";CONSTRAINT;\"REFTEXT\";\"CLASSEMENT\";\"ExisteDansSlayer\"\n" );
    for( i = 1 ; i < nn ; i++ ) {
        tokenlen = strlen( database[i].token );
        spacer[0]='"';
        for( j=1;j<20-tokenlen;j++) spacer[j]=' ';
        spacer[j]='"';
        spacer[j+1]='\0';
        if( database[i].reftext == NULL ) _nullref++;
#ifdef JAPANESE
        if( database[i].localize ) {                /* 日本語有り？  */
            if( database[i].reftext == NULL ) {     /* 原文なし？    */
                printf( "%s;%s;\"%s\";;%s;;\n", database[i].token, spacer, database[i].comment, database[i].localize );
            } else {                                /* 原文あり？    */
                if( !strcmp( database[i].localize, database[i].reftext )) {
                    printf( "%s;%s;\"%s\";;%s;;\n", database[i].token, spacer, database[i].comment, database[i].localize );
                } else {
                    printf( "%s;%s;\"%s\";;%s;%s;\n", database[i].token, spacer, database[i].comment, database[i].localize, database[i].reftext );
                }
            }
        } else if( database[i].reftext != NULL ) {  /* 日本語なし、原文あり？ */
#ifdef ENGLISH
            printf( "%s;%s;\"%s\";;%s;;\n", database[i].token, spacer, database[i].comment, database[i].reftext );
#endif
        }
#else
        if( database[i].reftext ) {
            printf( "%s;%s;\"%s\";;%s;;\n", database[i].token, spacer, database[i].comment, database[i].reftext );
        }
#endif
    }
    fprintf( stderr, "Total=%d, Patch=%d, Match=%d:%d, NullReftext=%d\n", _line, _patch, _match, _match_sd2, _nullref );
}
