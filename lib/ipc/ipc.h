#ifndef IPC_H_
#define IPC_H_

#include <stdint.h>
#include <sys/epoll.h>
#include <yajl/yajl_gen.h>

#include "IPCClient.h"

// clang-format off
#define IPC_MAGIC "DUSK-IPC"
#define IPC_MAGIC_ARR { 'D', 'U', 'S', 'K', '-', 'I', 'P', 'C'}
#define IPC_MAGIC_LEN 8 // Not including null char

#define IPCCOMMAND(FUNC, TYPE)                                                \
  { #FUNC, {.single_param=FUNC }, 1, (ArgType[1]){TYPE} }

#define IPCCOMMANDS(FUNC, ARGC, ...)                                          \
  { #FUNC, {.array_param=FUNC }, ARGC, (ArgType[ARGC]){__VA_ARGS__} }
// clang-format on

typedef enum IPCMessageType {
  IPC_TYPE_RUN_COMMAND = 0,
  IPC_TYPE_GET_MONITORS = 1,
  IPC_TYPE_GET_WORKSPACES = 2,
  IPC_TYPE_GET_LAYOUTS = 3,
  IPC_TYPE_GET_CLIENT = 4,
  IPC_TYPE_GET_SETTINGS = 5,
  IPC_TYPE_EVENT = 6
} IPCMessageType;

/**
 * Every IPC packet starts with this structure
 */
typedef struct dusk_ipc_header {
  uint8_t magic[IPC_MAGIC_LEN];
  uint32_t size;
  uint8_t type;
} __attribute((packed)) dusk_ipc_header_t;

typedef enum ArgType {
  ARG_TYPE_NONE = 0,
  ARG_TYPE_UINT = 1,
  ARG_TYPE_SINT = 2,
  ARG_TYPE_FLOAT = 3,
  ARG_TYPE_PTR = 4,
  ARG_TYPE_STR = 5
} ArgType;

/**
 * An IPCCommand function can have either of these function signatures
 */
typedef union ArgFunction {
  void (*single_param)(const Arg *);
  void (*array_param)(const Arg *, int);
} ArgFunction;

typedef struct IPCCommand {
  char *name;
  ArgFunction func;
  unsigned int argc;
  ArgType *arg_types;
} IPCCommand;

typedef struct IPCParsedCommand {
  char *name;
  Arg *args;
  ArgType *arg_types;
  unsigned int argc;
} IPCParsedCommand;

/**
 * Initialize the IPC socket and the IPC module
 *
 * @param socket_path Path to create the socket at
 * @param epoll_fd File descriptor for epoll
 * @param commands Address of IPCCommands array defined in config.h
 * @param commands_len Length of commands[] array
 *
 * @return int The file descriptor of the socket if it was successfully created,
 *   -1 otherwise
 */
int ipc_init(const char *socket_path, const int p_epoll_fd,
             IPCCommand commands[], const int commands_len);

/**
 * Uninitialize the socket and module. Free allocated memory and restore static
 * variables to their state before ipc_init
 */
void ipc_cleanup();

/**
 * Get the file descriptor of the IPC socket
 *
 * @return int File descriptor of IPC socket, -1 if socket not created.
 */
int ipc_get_sock_fd();

/**
 * Get address to IPCClient with specified file descriptor
 *
 * @param fd File descriptor of IPC Client
 *
 * @return Address to IPCClient with specified file descriptor, -1 otherwise
 */
IPCClient *ipc_get_ipc_client(int fd);

/**
 * Check if an IPC client exists with the specified file descriptor
 *
 * @param fd File descriptor
 *
 * @return int 1 if client exists, 0 otherwise
 */
int ipc_is_client_registered(int fd);

/**
 * Disconnect an IPCClient from the socket and remove the client from the list
 *   of known connected clients
 *
 * @param c Address of IPCClient
 *
 * @return 0 if the client's file descriptor was closed successfully, the
 * result of executing close() on the file descriptor otherwise.
 */
int ipc_drop_client(IPCClient *c);

/**
 * Accept an IPC Client requesting to connect to the socket and add it to the
 *   list of clients
 *
 * @return File descriptor of new client, -1 on error
 */
int ipc_accept_client();

/**
 * Read an incoming message from an accepted IPC client
 *
 * @param c Address of IPCClient
 * @param msg_type Address to IPCMessageType variable which will be assigned
 *   the message type of the received message
 * @param msg_size Address to uint32_t variable which will be assigned the size
 *   of the received message
 * @param msg Address to char* variable which will be assigned the address of
 *   the received message. This must be freed using free().
 *
 * @return 0 on success, -1 on error reading message, -2 if reading the message
 * resulted in EAGAIN, EINTR, or EWOULDBLOCK.
 */
int ipc_read_client(IPCClient *c, IPCMessageType *msg_type, uint32_t *msg_size,
                    char **msg);

/**
 * Write any pending buffer of the client to the client's socket
 *
 * @param c Client whose buffer to write
 *
 * @return Number of bytes written >= 0, -1 otherwise. errno will still be set
 * from the write operation.
 */
ssize_t ipc_write_client(IPCClient *c);

/**
 * Prepare a message in the specified client's buffer.
 *
 * @param c Client to prepare message for
 * @param msg_type Type of message to prepare
 * @param msg_size Size of the message in bytes. Should not exceed
 *   MAX_MESSAGE_SIZE
 * @param msg Message to prepare (not including header). This pointer can be
 *   freed after the function invocation.
 */
void ipc_prepare_send_message(IPCClient *c, const IPCMessageType msg_type,
                              const uint32_t msg_size, const char *msg);

/**
 * Prepare an error message in the specified client's buffer
 *
 * @param c Client to prepare message for
 * @param msg_type Type of message
 * @param format Format string following vsprintf
 * @param ... Arguments for format string
 */
void ipc_prepare_reply_failure(IPCClient *c, IPCMessageType msg_type,
                               const char *format, ...);

/**
 * Prepare a success message in the specified client's buffer
 *
 * @param c Client to prepare message for
 * @param msg_type Type of message
 */
void ipc_prepare_reply_success(IPCClient *c, IPCMessageType msg_type);

/**
 * Handle an epoll event caused by a registered IPC client. Read, process, and
 * handle any received messages from clients. Write pending buffer to client if
 * the client is ready to receive messages. Drop clients that have sent an
 * EPOLLHUP.
 *
 * @param ev Associated epoll event returned by epoll_wait
 * @param mons Address of Monitor pointing to start of linked list
 * @param lastselmon Address of pointer to previously selected monitor
 * @param selmon Address of selected Monitor
 * @param num_workspaces number of workspaces
 * @param layouts Array of available layouts
 * @param layouts_len Length of layouts array
 *
 * @return 0 if event was successfully handled, -1 on any error receiving
 * or handling incoming messages or unhandled epoll event.
 */
int ipc_handle_client_epoll_event(struct epoll_event *ev, Monitor *mons,
                                  Monitor **lastselmon, Monitor *selmon, const int num_workspaces,
                                  const Layout *layouts, const int layouts_len);

/**
 * Handle an epoll event caused by the IPC socket. This function only handles an
 * EPOLLIN event indicating a new client requesting to connect to the socket.
 *
 * @param ev Associated epoll event returned by epoll_wait
 *
 * @return 0, if the event was successfully handled, -1 if not an EPOLLIN event
 * or if a new IPC client connection request could not be accepted.
 */
int ipc_handle_socket_epoll_event(struct epoll_event *ev);

#endif /* IPC_H_ */
