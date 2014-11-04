/*
* Copyright (c) 2013 Digi International Inc.,
* All rights not expressly granted are reserved.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/.
*
* Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
* =======================================================================
*/

#include <stdlib.h>
#include "connector_api.h"
#include "platform.h"
#include <unistd.h>
#include <pthread.h>

#include <string.h>
#include <errno.h>

#include "device_request.h" /* Include the header files for device request */

/* Import the test functions to execute in the test cases list */
extern void * app_start_test_case_datapoints_loop(void * args);


/* Internal test structure to save and manage the device request */
typedef struct device_request_handle {
    char * payload;                       // Contains the data that will be sent in the response
    char const * target;                  // Contains the target
    size_t length_in_bytes;               // Stores the received target
} device_request_handle_t;

/* List of known targets for this test case */
static char * const device_request_targets[] = {
    "Data point",
    "test_datapoint_send_datastream_with_datapoints"
};
/* Internal test structure to create a list of arguments obtained from the device request payload */
typedef struct parsed_payload{
    char * element;
    void * next;
} parsed_payload_t;


/* Auxiliar functions to manage the payload */
// parsed_payload_t * getParsedPayload(const char * payload, char delimiter);
int split (char *str, char c, char ***arr);
// char** getParsedPayload(char * payload, char delimiter);
void freeQueue(parsed_payload_t * queue);











/* This function is called only one time for each target (First step) */
static connector_callback_status_t app_process_device_request_target(connector_data_service_receive_target_t * const target_data)
{
    connector_callback_status_t status = connector_callback_continue;

    APP_DEBUG("Device request data: target = \"%s\"\n", target_data->target);

    device_request_handle_t * device_request = target_data->user_context;

    if (target_data->user_context == NULL)
    {
        /* 1st chunk of device request so let's allocate memory for it
        * and setup user_context for the client_device_request.
        */
        {
            void * ptr;

            ptr = malloc(sizeof *device_request);
            if (ptr == NULL)
            {
                /* no memory so cancel this request */
                APP_DEBUG("app_process_device_request: malloc fails for device request \"%s\" target\n", target_data->target);
                status = connector_callback_error;
                goto done;
            }

            device_request = ptr;
            target_data->user_context = ptr;
        }
    }

    /* check for supported target */
    for(unsigned int i = 0; i < asizeof(device_request_targets); i++){

        if(strcmp(target_data->target, device_request_targets[i]) == 0){
            /* If it is a known target, fill the test structure */
            APP_DEBUG("Target data: %s\n", target_data->target);
            device_request->length_in_bytes = 0;
            device_request->payload = NULL;
            device_request->target = device_request_targets[i];
            goto done;
        }
    }

    /* Target not supported, so assign device_request_unsupported_target and send an error */
    device_request->target = NULL;
    status = connector_callback_error;

done:
    return status;
}





/* This function is called several times, one for each received packet */
static connector_callback_status_t app_process_device_request_data(connector_data_service_receive_data_t * const receive_data)
{
    connector_callback_status_t status = connector_callback_continue;
    device_request_handle_t * const device_request = receive_data->user_context;

    if(device_request->target != NULL)
    {
        /*
        * Next lines process the packet data and add them to "device_request->payload"
        */
        device_request->length_in_bytes += receive_data->bytes_used;
        device_request->payload = realloc(device_request->payload, device_request->length_in_bytes);
        memcpy(device_request->payload + device_request->length_in_bytes - receive_data->bytes_used, receive_data->buffer, receive_data->bytes_used);

        /* No more chunk */
        /* Sets bytes_sent counter to 0 and resets the firstPacket variable */
        if (!receive_data->more_data)
        {
            /* ONLY FOR DEBUG */
            APP_DEBUG("Total Payload received = \"%.*s\" for %s\n", device_request->length_in_bytes, device_request->payload, device_request->target);
//             APP_DEBUG("Total Payload received with length = \"%d\" for %s\n", device_request->length_in_bytes, device_request->target);
        }
    }
    else
    {
        status = connector_callback_error;

        if (device_request != NULL && device_request->payload != NULL)
        {
            free(device_request->payload);
            device_request->payload = NULL;
        }
    }

    return status;
}



/* This function is called several times, one for each packet to send in the response */
static connector_callback_status_t app_process_device_request_response(connector_data_service_receive_reply_data_t * const reply_data)
{
    connector_callback_status_t status = connector_callback_continue;
    device_request_handle_t * const device_request = reply_data->user_context;
    size_t bytes_used = 0;
    void const * buffer = NULL;

    if(device_request->target == NULL)
    {
        // If the target isn't supported, sends the message below
        static char const unsupported_target_text[] = "Target is not supported";
        buffer = unsupported_target_text;
        bytes_used = sizeof unsupported_target_text -1;
    }
    else
    {
        static char const request_successful[] = "Launch successful";
        buffer = request_successful;
        bytes_used = sizeof request_successful -1;
    }

    /* Send the buffer content */
    ASSERT(reply_data->bytes_available >= bytes_used);

    /* let's copy the response data to service_response buffer */
    memcpy(reply_data->buffer, buffer, bytes_used);
    reply_data->bytes_used = bytes_used;


    return status;
}




/* this function is called at the final of the process */
static connector_callback_status_t app_process_device_request_status(connector_data_service_status_t const * const status_data)
{
    connector_callback_status_t status = connector_callback_continue;
    device_request_handle_t * const device_request = status_data->user_context;

    if (device_request != NULL)
    {
        switch (status_data->status)
        {
        case connector_data_service_status_complete:
            APP_DEBUG("app_process_device_request_status: handle %p session completed\n",
                    (void *) status_data->user_context);
            break;
        case connector_data_service_status_session_error:
            APP_DEBUG("app_process_device_request_status: handle %p session error %d\n",
                    (void *) status_data->user_context, status_data->session_error);
            break;
        default:
            APP_DEBUG("app_process_device_request_status: handle %p session error %d\n",
                        (void *)status_data->user_context, status_data->status);
            break;
        }
    }

    return status;
}






connector_callback_status_t app_data_service_handler(connector_request_id_data_service_t const request_id, void * const data)
{
    connector_callback_status_t status = connector_callback_continue;
    switch (request_id)
    {
        case connector_request_id_data_service_receive_target:
            status = app_process_device_request_target(data);
            break;
        case connector_request_id_data_service_receive_data:
            status = app_process_device_request_data(data);
            break;
        case connector_request_id_data_service_receive_status:
            /* Call to manage the request status */
            status = app_process_device_request_status(data);

            /******************************************************/
            /*** Manage the actions requested by Device Request ***/
            /******************************************************/
            device_request_handle_t * const device_request = ((connector_data_service_status_t*)data)->user_context;

            if(device_request->target != NULL){
                /* If the target is supported, manage the payload and do the corresponding action */

                    /* Create a string null terminated to process the payload */
                    char * payload = malloc( sizeof(char) * (device_request->length_in_bytes + 1) );
                    memcpy(payload, device_request->payload, device_request->length_in_bytes);
                    payload[device_request->length_in_bytes] = '\0';

//                     printf("Payload before the split: '%s'\n", payload);

                    /* Split the payload into the arguments */
                    char **arguments = NULL;
                    char delimiter = ';';
                    int numberOfArguments = 0;
                    numberOfArguments = split(payload, delimiter, &arguments);

//                     printf("found %d tokens.\n", numberOfArguments);
// 
//                     for (int i = 0; i < numberOfArguments; i++)
//                         printf("string #%d: %s\n", i, arguments[i]);


                /* Test cases from test_put_request_positive.py */
                if(strcmp(device_request->target, "test_datapoint_send_datastream_with_datapoints") == 0)
                {
                    /* If the target is Data point, we know that there are 2 elements in the string */
                    unsigned int numberOfLoops = atoi(arguments[0]); /* Obtain an Integer from a char[] */
                    unsigned int numberStreams = atoi(arguments[1]); /* Obtain an Integer from a char[] */
                    unsigned int numberPointsPerStream = atoi(arguments[2]); /* Obtain an Integer from a char[] */
                    char const * valueType = arguments[3];


                    /* Execute the TEST_CASE in a new thread **********************/
                    /* Create internal structure to save the test arguments */
                    test_thread_arguments_t * arguments = malloc(sizeof(test_thread_arguments_t));
                    arguments->numberOfLoops = numberOfLoops;
                    arguments->numberPointsPerStream = numberPointsPerStream;
                    arguments->numberStreams = numberStreams;
                    arguments->valueType = malloc( (sizeof(char) * strlen(valueType)) + 1);
                    sprintf(arguments->valueType, "%s", valueType);
                    arguments->length_bytes = strlen(valueType);

                    /* Create a new thread to execute the test case with the arguments */
                    pthread_t test_case_thread;
                    int ccode = pthread_create(&test_case_thread, NULL, app_start_test_case_datapoints_loop, (void *)arguments);
                    if (ccode != 0)
                    {
                        APP_DEBUG("thread_create() app_start_test_case_datapoints_loop on data_point.c %d\n", ccode);
                    }

                }

                /* Release the memory allocated for the queue */
                for (int i = 0; i < numberOfArguments; i++)
                    free(arguments[i]);
                free(arguments);
                free(payload);
            }


            /* Free internal structure for device request */
            if(device_request != NULL){
                if( device_request->payload != NULL )
                {
                    free(device_request->payload);
                }

                free(device_request);
            }
            else{
                APP_DEBUG("device_request is null");
            }

            break;
        case connector_request_id_data_service_receive_reply_data:
            status = app_process_device_request_response(data);
            break;
        case connector_request_id_data_service_receive_reply_length:
        default:
            APP_DEBUG("app_data_service_handler: unknown request id type %d for connector_request_id_data_service\n", request_id);
            break;
    }

    return status;
}




/**
 *  splits str on delim and dynamically allocates an array of pointers.
 *
 *  On error -1 is returned, check errno
 *  On success size of array is returned, which may be 0 on an empty string
 *  or 1 if no delim was found.
 *
 *  You could rewrite this to return the char ** array instead and upon NULL
 *  know it's an allocation problem but I did the triple array here.  Note that
 *  upon the hitting two delim's in a row "foo,,bar" the array would be:
 *  { "foo", NULL, "bar" }
 *
 *  You need to define the semantics of a trailing delim Like "foo," is that a
 *  2 count array or an array of one?  I choose the two count with the second entry
 *  set to NULL since it's valueless.
 *  Modifies str so make a copy if this is a problem
 */
int split (char *str, char c, char ***arr)
{
    int count = 1;
    int token_len = 1;
    int i = 0;
    char *p;
    char *t;

    p = str;
    while (*p != '\0')
    {
        if (*p == c)
            count++;
        p++;
    }

    *arr = (char**) malloc(sizeof(char*) * count);
    if (*arr == NULL)
        exit(1);

    p = str;
    while (*p != '\0')
    {
        if (*p == c)
        {
            (*arr)[i] = (char*) malloc( sizeof(char) * token_len + 1 );
            if ((*arr)[i] == NULL)
                exit(1);

            token_len = 0;
            i++;
        }
        p++;
        token_len++;
    }
    (*arr)[i] = (char*) malloc( sizeof(char) * token_len + 1 );
    if ((*arr)[i] == NULL)
        exit(1);

    i = 0;
    p = str;
    t = ((*arr)[i]);
    while (*p != '\0')
    {
        if (*p != c && *p != '\0')
        {
            *t = *p;
            t++;
        }
        else
        {
            *t = '\0';
            i++;
            t = ((*arr)[i]);
        }
        p++;
    }

    return count;
}






void freeQueue(parsed_payload_t * queue){

    parsed_payload_t * auxiliar = queue;

    while(auxiliar != NULL){
        parsed_payload_t * next = auxiliar->next;
        free(auxiliar->element);
        free(auxiliar);
        auxiliar = next;
    }
    //free(queue);
    queue = NULL;
}


// void freeQueue(parsed_payload_t * queue){
// 
//     if ( queue != NULL )
//     {
//         /* Free recursive list */
//         freeQueue(queue->next);
//         if ( queue->next != NULL )
//         {
//             free(queue->next);
//         }
// 
//         /* Free element */
//         if ( queue->element != NULL )
//         {
//             free(queue->element);
//         }
//     }
// 
//     return;
// 
// 
// }


