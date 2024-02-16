format ELF64 executable

include "linux.inc"

MAX_CONN equ 5
REQUEST_CAP equ 128*1024

segment readable executable

include "utils.inc"

entry main
main:
    funcall2 write_cstr, STDOUT, start

    funcall2 write_cstr, STDOUT, socket_trace_msg
    socket AF_INET, SOCK_STREAM, 0
    cmp rax, 0
    jl .fatal_error
    mov qword [sockfd], rax

    setsockopt [sockfd], SOL_SOCKET, SO_REUSEADDR, enable, 4
    cmp rax, 0
    jl .fatal_error

    setsockopt [sockfd], SOL_SOCKET, SO_REUSEPORT, enable, 4
    cmp rax, 0
    jl .fatal_error

    funcall2 write_cstr, STDOUT, bind_trace_msg
    mov word [servaddr.sin_family], AF_INET
    mov word [servaddr.sin_port], 14619
    mov dword [servaddr.sin_addr], INADDR_ANY
    bind [sockfd], servaddr.sin_family, sizeof_servaddr
    cmp rax, 0
    jl .fatal_error

    funcall2 write_cstr, STDOUT, listen_trace_msg
    listen [sockfd], MAX_CONN
    cmp rax, 0
    jl .fatal_error

.next_request:
    funcall2 write_cstr, STDOUT, accept_trace_msg
    accept [sockfd], cliaddr.sin_family, cliaddr_len
    cmp rax, 0
    jl .fatal_error

    mov qword [connfd], rax

    read [connfd], request, REQUEST_CAP
    cmp rax, 0
    jl .fatal_error
    mov [request_len], rax

    mov [request_cur], request

    write STDOUT, [request_cur], [request_len]

    funcall4 starts_with, [request_cur], [request_len], get, get_len
    cmp rax, 0
    jg .handle_get_method

    funcall4 starts_with, [request_cur], [request_len], post, post_len
    cmp rax, 0
    jg .handle_post_method

    jmp .serve_error_405

.handle_get_method:
    add [request_cur], get_len
    sub [request_len], get_len

    funcall4 starts_with, [request_cur], [request_len], index_route, index_route_len
    call starts_with
    cmp rax, 0
    jg .serve_index_page

    jmp .serve_error_404

.handle_post_method:
    add [request_cur], post_len
    sub [request_len], post_len

    funcall4 starts_with, [request_cur], [request_len], index_route, index_route_len
    cmp rax, 0
    jg .process_run_or_stop

    funcall4 starts_with, [request_cur], [request_len], shutdown_route, shutdown_route_len
    cmp rax, 0
    jg .process_shutdown

    jmp .serve_error_404

.process_shutdown:
    funcall2 write_cstr, [connfd], shutdown_response
    jmp .shutdown

.process_run_or_stop:
    call drop_http_header
    cmp rax, 0
    je .serve_error_400

    funcall4 starts_with, [request_cur], [request_len], start_form_data_prefix, start_form_data_prefix_len
    cmp rax, 0
    jg .start_and_serve_index_page

.start_and_serve_index_page:
    funcall2 write_cstr, STDOUT, start_msg
    jmp .serve_index_page

.serve_index_page:
    funcall2 write_cstr, [connfd], index_page_response
    funcall2 write_cstr, [connfd], index_page_header
    funcall2 write_cstr, [connfd], index_page_footer
    close [connfd]
    jmp .next_request

.serve_error_400:
    funcall2 write_cstr, [connfd], error_400
    close [connfd]
    jmp .next_request

.serve_error_404:
    funcall2 write_cstr, [connfd], error_404
    close [connfd]
    jmp .next_request

.serve_error_405:
    funcall2 write_cstr, [connfd], error_405
    close [connfd]
    jmp .next_request

.shutdown:
    funcall2 write_cstr, STDOUT, ok_msg
    close [connfd]
    close [sockfd]
    exit 0

.fatal_error:
    funcall2 write_cstr, STDERR, error_msg
    close [connfd]
    close [sockfd]
    exit 1

drop_http_header:
.next_line:
    funcall4 starts_with, [request_cur], [request_len], clrs, 2
    cmp rax, 0
    jg .reached_end

    funcall3 find_char, [request_cur], [request_len], 10
    cmp rax, 0
    je .invalid_header

    mov rsi, rax
    sub rsi, [request_cur]
    inc rsi
    add [request_cur], rsi
    sub [request_len], rsi

    jmp .next_line

.reached_end:
    add [request_cur], 2
    sub [request_len], 2
    mov rax, 1
    ret

.invalid_header:
    xor rax, rax
    ret

.error:
   close [rsp+8]
   add rsp, 16
   ret

segment readable writeable

enable dd 1
sockfd dq -1
connfd dq -1
servaddr servaddr_in
sizeof_servaddr = $ - servaddr.sin_family
cliaddr servaddr_in
cliaddr_len dd sizeof_servaddr

clrs db 13, 10

error_400            db "HTTP/1.1 400 Bad Request", 13, 10
                     db "Content-Type: text/html; charset=utf-8", 13, 10
                     db "Connection: close", 13, 10
                     db 13, 10
                     db "<h1>Bad Request</h1>", 10
                     db "<a href='/'>Back to Home</a>", 10
                     db 0
error_404            db "HTTP/1.1 404 Not found", 13, 10
                     db "Content-Type: text/html; charset=utf-8", 13, 10
                     db "Connection: close", 13, 10
                     db 13, 10
                     db "<h1>Page not found</h1>", 10
                     db "<a href='/'>Back to Home</a>", 10
                     db 0
error_405            db "HTTP/1.1 405 Method Not Allowed", 13, 10
                     db "Content-Type: text/html; charset=utf-8", 13, 10
                     db "Connection: close", 13, 10
                     db 13, 10
                     db "<h1>Method not Allowed</h1>", 10
                     db "<a href='/'>Back to Home</a>", 10
                     db 0
index_page_response  db "HTTP/1.1 200 OK", 13, 10
                     db "Content-Type: text/html; charset=utf-8", 13, 10
                     db "Connection: close", 13, 10
                     db 13, 10
                     db 0
index_page_header    db "<h1>Nice things halt!</h1>", 10
                     db "<script>", 10
                     db "document.addEventListener('DOMContentLoaded', function () {", 10
                     db "    var canvas = document.getElementById('gridCanvas');", 10
                     db "    var context = canvas.getContext('2d');", 10
                     db "    var gridSize = 40;", 10
                     db "    var canvasWidth = canvas.width;", 10
                     db "    var canvasHeight = canvas.height;", 10
                     db "    var cellSize = canvasWidth / gridSize;", 10
                     db "    for (var x = 0; x <= canvasWidth; x += cellSize) {", 10
                     db "        for (var y = 0; y <= canvasHeight; y += cellSize) {", 10
                     db "            context.rect(x, y, cellSize, cellSize);", 10
                     db "        }", 10
                     db "    }", 10
                     db "    context.strokeStyle = '#ddd';", 10
                     db "    context.stroke();", 10
                     db "    canvas.addEventListener('click', function (event) {", 10
                     db "        var clickedX = Math.floor(event.offsetX / cellSize) * cellSize;", 10
                     db "        var clickedY = Math.floor(event.offsetY / cellSize) * cellSize;", 10
                     db "        context.fillStyle = '#000';", 10
                     db "        context.fillRect(clickedX, clickedY, cellSize, cellSize);", 10
                     db "    });", 10
                     db "});", 10
		     db "	function submitCanvasData() {", 10
		     db "            console.log('in submit canvas data')", 10 
		     db "            var canvas = document.getElementById('gridCanvas');", 10
		     db "            var context = canvas.getContext('2d');", 10
		     db "            var gridSize = 40;", 10
		     db "            var cellSize = canvas.width / gridSize;", 10
		     db "            var dataArray = [];", 10
		     db "", 10
		     db "            for (var x = 0; x < gridSize; x++) {", 10
		     db "                for (var y = 0; y < gridSize; y++) {", 10
		     db "                    var pixelData = context.getImageData(x * cellSize, y * cellSize, 1, 1).data;", 10
		     db "                    // Check if the pixel is black", 10
		     db "                    var isBlack = pixelData[0] === 0 && pixelData[1] === 0 && pixelData[2] === 0;", 10
		     db "                    dataArray.push(isBlack ? 1 : 0);", 10
		     db "                }", 10
		     db "            }", 10
		     db "", 10
		     db "            // Convert the array to a string", 10
		     db "            var dataString = dataArray.join('');", 10
		     db "", 10
		     db "            // Create a hidden input field to store the data", 10
		     db "            var hiddenInput = document.createElement('input');", 10
		     db "            hiddenInput.type = 'hidden';", 10
		     db "            hiddenInput.name = 'canvasData';", 10
		     db "            hiddenInput.value = dataString;", 10
		     db "", 10
		     db "            // Append the hidden input to the form", 10
		     db "            var form = document.getElementById('runForm');", 10
		     db "            form.appendChild(hiddenInput);", 10
	             db "            console.log(dataString)", 10
		     db "            form.submit();", 10
		     db "        }", 10
                     db "</script>", 10
                     db "<canvas id='gridCanvas' width='1000' height='520'></canvas>", 10
                     db "<form method='post' action='/shutdown'>", 10
                     db "    <input type='submit' value='shutdown'>", 10
                     db "</form>", 10
                     db "<form style='display: inline' method='post' id='runForm' action='/'>", 10
                     db "<button type='submit' onclick='submitCanvasData()' name='start' value=''>Run</button>", 10
		     db "<input type='hidden' name='canvasData' id='canvasData'></form>"
                     db 0
index_page_footer    db 0
shutdown_response    db "HTTP/1.1 200 OK", 13, 10
                     db "Content-Type: text/html; charset=utf-8", 13, 10
                     db "Connection: close", 13, 10
                     db 13, 10
                     db "<h1>Shutting down the server...</h1>", 10
                     db "Please close this tab"
                     db 0
start_msg db "in start", 10, 0

start_form_data_prefix db "start="
start_form_data_prefix_len = $ - start_form_data_prefix

get db "GET "
get_len = $ - get
post db "POST "
post_len = $ - post

index_route db "/ "
index_route_len = $ - index_route

shutdown_route db "/shutdown "
shutdown_route_len = $ - shutdown_route

starting_route db "/run "
starting_route_len = $ - starting_route

start            db "INFO: Starting Web Server!", 10, 0
ok_msg           db "INFO: OK!", 10, 0
socket_trace_msg db "INFO: Creating a socket...", 10, 0
bind_trace_msg   db "INFO: Binding the socket...", 10, 0
listen_trace_msg db "INFO: Listening to the socket...", 10, 0
accept_trace_msg db "INFO: Waiting for client connections...", 10, 0
error_msg        db "FATAL ERROR!", 10, 0

request_len rq 1
request_cur rq 1
request     rb REQUEST_CAP

;; [todo][todo][todo][todo][todo][todo]...
;;             ^
todo_end_offset rq 1

statbuf rb sizeof_stat64
