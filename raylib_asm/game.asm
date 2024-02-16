format ELF64

section '.text' executable

public _start
extrn printf
extrn getpid
extrn _exit
extrn InitWindow
extrn WindowShouldClose
extrn CloseWindow
extrn BeginDrawing
extrn EndDrawing
extrn DrawText
extrn IsCursorOnScreen
extrn ClearBackground
extrn SetTargetFPS
extrn DrawRectangle

_start:
	mov rdi, msg
	call printf
	
	mov rdi, 300
	mov rsi, 300
	mov rdx, title
        call InitWindow

	mov rdi, 30
	call SetTargetFPS

.again:
	call WindowShouldClose
	test rax, rax
	jnz .over

	call BeginDrawing

	mov rdi, 0x0
	call ClearBackground

	call IsCursorOnScreen
	test rax, rax
	jnz .draw_fps

	mov edi, dword [x]
	mov esi, dword [y]
	mov rdx, 100
	mov rcx, 100
	mov r8, 0xFF0000FF
	call DrawRectangle

	inc dword [x]
	inc dword [y]

	call EndDrawing

	jmp .again

.draw_fps:
	call WindowShouldClose
	test rax, rax
	jnz .over

	call BeginDrawing

	mov [x], 10
	mov [y], 10

	mov rdi, 0x0
	call ClearBackground

	mov rdi, title
	mov rsi, 300
	mov rdx, 300
	mov rcx, 50
	mov r8, 0xFF0000FF
	call DrawText
	
	call EndDrawing

	jmp .again

.over:
	call CloseWindow
        mov rdi, 0
        call _exit

section '.data' writeable
 image_path db "./test.png", 0
 msg db "Hello World!", 0  
 msg_no db "Yes", 0  
 title db "This is a test", 0
 x dd 10
 y dd 10
