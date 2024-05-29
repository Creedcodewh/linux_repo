#include "head.h"
#include "l8w8jwt/decode.h"
#include "l8w8jwt/encode.h"

int encode(char *key, char *user_name, char *token)
{
    char* jwt;
    size_t jwt_length;

    struct l8w8jwt_encoding_params params;
    l8w8jwt_encoding_params_init(&params);

    params.alg = L8W8JWT_ALG_HS512;

    params.sub = user_name;
    // params.iss = "Black Mesa";
    //params.aud = "Administrator";

    params.iat = time(NULL);
    params.exp = time(NULL) + 600; /* Set to expire after 10 minutes (600 seconds). */

    params.secret_key = (unsigned char*)key;
    params.secret_key_length = strlen(key);

    params.out = &jwt;
    params.out_length = &jwt_length;

    int r = l8w8jwt_encode(&params);

    // printf("strlen(jwd) = %lu\n", strlen(jwt));
    // printf("\n l8w8jwt example HS512 token: %s \n", r == L8W8JWT_SUCCESS ? jwt : " (encoding failure) ");
    if(r != L8W8JWT_SUCCESS)
    {
        printf("encoding failure\n");
        return EXIT_FAILURE;
    }
    strcpy(token, jwt);
    /* Always free the output jwt string! */
    l8w8jwt_free(jwt);

    return 0;
}

int decode(char *key, char *user_name, char *token)
{
    struct l8w8jwt_decoding_params params;
    l8w8jwt_decoding_params_init(&params);

    params.alg = L8W8JWT_ALG_HS512;

    params.jwt = token;
    params.jwt_length = strlen(token);    
    params.verification_key = (unsigned char*)key;
    params.verification_key_length = strlen(key);

    /* 
     * Not providing params.validate_iss_length makes it use strlen()
     * Only do this when using properly NUL-terminated C-strings! 
     */
    // params.validate_iss = "Black Mesa"; 
    params.validate_sub = user_name;

    /* Expiration validation set to false here only because the above example token is already expired! */
    params.validate_exp = 1; 
    params.exp_tolerance_seconds = 60;

    params.validate_iat = 1;
    params.iat_tolerance_seconds = 60;

    enum l8w8jwt_validation_result validation_result;

    int decode_result = l8w8jwt_decode(&params, &validation_result, NULL, NULL);

    if (decode_result == L8W8JWT_SUCCESS && validation_result == L8W8JWT_VALID) 
    {
        printf("\n Example HS512 token validation successful! \n");
    }
    else
    {
        printf("\n Example HS512 token validation failed! \n");
        return EXIT_FAILURE;
    }
    
    /*
     * decode_result describes whether decoding/parsing the token succeeded or failed;
     * the output l8w8jwt_validation_result variable contains actual information about
     * JWT signature verification status and claims validation (e.g. expiration check).
     * 
     * If you need the claims, pass an (ideally stack pre-allocated) array of struct l8w8jwt_claim
     * instead of NULL,NULL into the corresponding l8w8jwt_decode() function parameters.
     * If that array is heap-allocated, remember to free it yourself!
     */

    return 0;
}
