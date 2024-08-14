# 운영체제 Assignment 4

### 이름 : 이준휘

### 학번 : 2018202046

### 교수 : 최상호 교수님

### 강의 시간 : 금 1, 2


## 1. Introduction

#### 해당 과제는 2 가지의 소과제로 나누어 진행된다. 첫 번째 과제에서는 PID를 바탕으로 프

```
로세스의 이름과 PID, 가상 메모리 주소, data, code, heap 주소와 원본 파일의 전체 경로
를 출력하는 모듈을 작성한다. 해당 과제는 2 차에서 작성한 ftrace를 wrapping하여 사용
한다. 두 번째 과제에서는 Dynamic Recompilation을 구현한다. 기존 컴파일된 함수를
shared memory와 memory mapping을 이용하여 가져온다. 가져온 함수는 make 실행 방
식에 따라 그대로 가져오거나, 최적화하여 가져온다. 가져온 두 함수의 성능을 5 0 set 비
교한다.
```
## 2. Conclusion & Analysis

## A. Assignment 4 - 1

해당 파일은 file_varea.c 파일로 ftrace를 wrapping하여 pid에 따른 메모리 주소 관련 정보와
파일 정보를 출력하는 모듈이 존재한다.
기존 ftrace의 위치를 저장할 real_ftrace가 존재하며, systemcall table의 주소를 저장할 변수가
있다.
file_varea 함수는 기존 ftrace를 hooking할 함수로 실제적인 모듈의 동작이 정의되어 있다. 입
력받은 pid의 값을 reg->dl의 위치에서 추출한다. 이후 pid_task 함수를 통해 입력받은 pid에
해당하는 task_struct의 위치를 가져와 findtask에 저장한다. 만약 find_task가 유효하지 않을 경
우 함수를 종료시킨다. 유효할 경우 해당 task_struct의 mm_struct를 get_task_mm 함수를 통해
위치를 추출한다. 그리고 mm_struct 내에서 존재하는 vm_area_struct를 mmap 변수를 통해 가
져온 후 출력을 시작함을 알린다.
While 문을 통해 vm이 유효한 동안 출력을 수행한다. 현재 file 정보를 file->vm_file에서 가져


온다. 만약 가져온 파일 정보가 유효하지 않은 경우 다음 vm을 탐색한다. 만약 유효한 경우
D_path를 통해 해당 파일의 path를 가져와 file_path에 저장한다. 이후 mem 주소는 vm-
>vm_start, vm->vm_end로, code의 주소는 mm->start_code, mm-end_code로, data의 주소는
mm->start_data, mm->end_data로, heap의 주소는 mm->start_brk, mm->brk로 출력하며,
file_path에 저장된 파일 위치를 출력한다.
Make_rw()함수는 system call table에 쓰기 권한을 부여한다.
Make_ro()함수는 system call table에 쓰기 권한을 뺏는다.
Module initialize 시 sys_call_table을 가져와 쓰기 권한을 부여한다. 이후 현재 ftrace 위치의 함
수를 real_trace에 저장하고, file_varea 함수를 해당 위치에 hooking한다. 이후 쓰기 권한을 뺏는
다.
Module exit 시 sys_call_table에 쓰기 권한을 부여하고, 원래 ftrace로 해당 system call을 변경한
다. 이후 쓰기 권한을 뺏는다.

해당 파일은 모듈을 만들기 위한 Makefile이다. 이전 2, 3 과제와 같은 방법으로 파일을 작성하
였다.


위의 함수는 과제에서 모듈을 적용하고 과제에서 주어진 test 파일을 수행하였을 때의 결과를
나타낸다. 위의 결과를 보면 task의 이름과 PID가 정상 출력되며 각 code 주소, heap 주소, data
주소, virtual memory의 주소와 해당 파일 명 또한 정확히 출력되는 모습을 확인할 수 있다. 이
를 통해 해당 과제가 목표에 맞게 구현되었음을 확인하였다.

## B. Assignment 4 - 2

해당 이미지는 D_recompile_test.c 파일을 .o 파일로 변환 후 이를 objdump -d 명령을 통해 코
드를 확인한 내용이다. 위의 코드에서 보았을 때 각 instruction 별로 길이가 다른 점을 확인할
수 있다. 우선 볼 부분은 우선 나눗셈의 인자로 쓰기는 dl을 설정하는 명령이다. 해당 명령은
B2 02를 통해 2 를 dl에 입력하는 것을 확인할 수 있다. 그 다음으로는 add 명령이다. Add 명령
은 83 c0 XX 와 같은 형식으로 XX 자리에 숫자를 입력하여 더한다. 다음으로 sub 명령은 83 E 8


XX 형태로 add와 비슷하게 이루어져 있다. Imul의 경우 06 C 0 XX의 형태로 이 또한 add와 sub
과 유사한 형태로 동작한다. 마지막으로 div의 경우 f6 f2로 dl의 값을 나누는 모습을 보인다.
이를 통해 x 86 에서 assembly language가 어떻게 생겼는지 확인하여 패턴을 특정할 수 있었다.

위의 파일은 shared memory에서 컴파일된 코드를 가져와 조건에 따라 재컴파일을 진행하는
D_recompile.c이다. 해당 파일에서는 전역변수로 저장할 재컴파일할 함수를 저장할 포인터
Operation과 shared memory의 데이터를 가리킬 compiled_code 포인터가 존재한다. 그리고
compiled_code의 shm_id를 저장할 segment id가 있다.

Sharedmem_init() 함수에서는 shmget()함수를 통해 D_recompile_test.c 파일에 key값으로 설정
된 1234 를 통해 shared_memory의 id를 가져온다. 이후 compiled_code가 가리키는 값을


shmat() 함수를 통해 segment_id가 가리키는 메모리로 attach한다.
Sharedmem_exit() 함수는 segment_id를 shmctl()의 IPC_RMID 명령을 사용하여 없애준다.

Drecompile_init() 함수는 mmap()함수를 사용하여 Operation에 임의의 메모리 공간을 mapping
해준다. 이 때 MAP_PRIVATE을 통해 해당 프로세스만 단독으로 사용하며, MAP_ANONYMOUS를
통해 다른 file descriptor를 사용하지 않고 생성할 수 있다. 권한은 RW로 제한한다.
Drecompile_exit()함수에서는 Operation의 위치를 활용하여 munmap()함수로 unmapping시켜준
다.

Drecompile() 함수는 shared_memory의 영역에서 mapping된 영역으로 데이터를 복사하는 과
정이 있다. #ifdef 문을 사용하여 dynamic 옵션이 존재하는 경우에는 optimized된 컴파일 복사
를 제공하며, 옵션이 없을 경우에는 단순히 0 xC3(종료 지점의 값)까지 Operation에 해당하는
func에 복사한다.
Dynamic 옵션이 적용되었을 때 0 xC3까지 다음 명령을 반복한다.
이전 코드를 보았을 때 0 xB2를 입력받은 경우 다음 값은 dl에 입력할 값이기 때문에(mov) 해
당 값을 임시로 저장한다.
만약 현재 컴파일 코드의 위치의 값이 0x 83 또는 0 x 68 인 경우 add sub imul 명령이기 때문에
3 가지 인자를 저장할 필요가 있다. 이에 prev_op에 해당 3 가지 상태를 모두 저장한다. 그 후
반복문을 통해 만약 기존 명령과 중복되는 명령일 경우 반복을 수행한다. 이 때 0x83은 덧셈
연산을(add or sub), 아닌 경우 곱셈 연산을 통해 prev_op[ 2 ] 값을 업데이트한다. 반복문을 탈출
한 경우 해당 코드를 func에 넣어준다.
만약 현재 컴파일 코드의 위치의 값이 0xF6인 경우 div 명령이기 때문에 2 가지 인자를 저장할
필요가 있다. Prev_op[ 1 ]에 dl의 값을 저장하며 이전과 마찬가지로 명령이 중복될 때 마다 dl의
값을 곱 누적하여 업데이트를 진행한다. Sub 명령의 경우 dl 레지스터의 값을 나누기 때문에
mov 명령을 통해 현재 prev_op[ 1 ]의 값을 dl reg에 넣어두며 이후 기존 명령과 같은 나눗셈 연
산을 진행한다.
이외의 경우에는 기존의 코드를 단순 복사한다.
#endif 이후 Operation의 권한을 RX로 변경하기 위해 mprotect() 함수를 사용한다. 그 후
Operation의 주소를 반환하며 함수를 종료한다.

Main 함수의 동작은 다음과 같다. Func 포인터는 실행 시에 사용할 포인터이며, struct에 변수의
값을 저장할 예정이다. Sharedmem_init()과 drecompile_init()을 통해 shared memory와 이를 저
장할 memory 공간을 준비한다. 이후 drecompile(Operation) 함수를 수행하며 이 결과를 func
이 가리키는 값으로 한다. 그 후 clock_gettime() 함수를 사용하여 시작 시간을 체크하며,
1 , 000 , 000 회 동안 func을 수행한다. 이후 해당 종료 시간을 체크하고, 시작시간과 종료 시간의
차이를 이용하여 수행 시간을 계산하여 출력한다. 이후 dercompile_exit()과 sharedmem_exit()
함수를 사용하여 각 메모리 공간을 할당 해제한다.


해당 파일은 Makefile로 기존의 주어진 형식 예시와 동일하게 작성되었다.

위의 결과는 shared memory를 할당하지 않고 수행할 경우 해당 영역이 잡히지 않아 failed가
뜨는 보습을 보인다.


다음은 이전의 gcc -o test2 D_recompile_test.c 명령을 통해 test2 프로그램을 생성하여, 이를 먼
저 수행한 후, 단순한 make를 통해 ./drecompile을 생성한 모습이다. ./test2를 통해
shared_memory에 공간이 할당되었으며 이후 ./drecompile 함수에서 정상적으로 함수가 수행되
어 결과가 나온 모습을 볼 수 있다. 수행 시간은 약 0.3 91709 초 소요되었으며 결과는 15 로 나
왔다.

다음은 make dynamic를 통해 ./drecompile을 생성한 모습이다. ./test2를 통해 shared_memory
에 공간이 할당되었으며 이후 ./drecompile 함수에서 정상적으로 함수가 수행되어 결과가 나온
모습을 볼 수 있다. 수행 시간은 약 0. 099794 초 소요되었으며 결과는 15 로 나왔다. 이는 이전
의 결과보다 시간이 매우 단축된 모습을 보였다.
Default
0.391709 0.390154 0.392357 0.391474 0.392061 0.395001 0.390843 0.392604 0.391756 0.
0.39485 0.394798 0.393754 0.400061 0.393499 0.394841 0.395131 0.394317 0.40101 0.
0.397048 0.394051 0.395587 0.39319 0.397692 0.395443 0.395986 0.395117 0.398752 0.


0.393827 0.392655 0.393901 0.397765 0.393109 0.394664 0.392269 0.393231 0.389908 0.
0.399879 0.393079 0.395849 0.394019 0.391127 0.394237 0.394473 0.392911 0.391879 0.
Dynamic
0.099794 0.098883 0.098023 0.099004 0.099363 0.099327 0.099502 0.1002 0.099207 0.
0.099495 0.098289 0.08798 0.099492 0.100736 0.100321 0.098316 0.098352 0.10014 0.
0.098585 0.099772 0.099452 0.099204 0.09956 0.098614 0.098699 0.102665 0.098196 0.
0.099576 0.099824 0.09848 0.099702 0.098717 0.099296 0.099042 0.099934 0.098974 0.
0.099777^ 0.099972^ 0.098465^ 0.098662^ 0.10131^ 0.099784^ 0.097939^ 0.09886^ 0.100616^ 0.^
Default의 50 set의 값은 다음과 같이 나왔으며 평균 0.39442374초가 소요되었다. 이에 반해
Dynamic 50 set의 값은 평균 0.09916904초가 나와 약 3.977287배의 성능 개선이 이루어졌다.
이에 해당 목표에 맞게 정확히 동작함을 확인하였다.

## 3. Consideration

```
해당 과제를 통해 task_struct 내에 존재하는 메모리가 어떠한 구조로 이루어져 있으며,
이를 통해 가상 메모리 주소와 실제 위치, 파일 경로 등을 추적하는 방법을 알 수 있었
다. 또한 shared memory와 관련한 함수에 대하여 익힐 수 있는 과제였다. 그리고 mmap
함수를 통해 특정 파일 혹은 가상의 메모리 공간을 할당하는 방식을 직접 실습할 수 있
었다. 마지막으로 프로그램을 objdump을 통해 자세히 확인하여 recompile을 하는 테크
닉을 익히 수 있는 과제였다. 다만 과제 중 기존에 주어진 Operation 함수가 1 이 아닌
값에서는 동작하지 않는다는 사실이 주어지지 않아 해당 부분에서 Floating Point
Exception으로 인해 많은 시간이 소요되었다.
```
## 4. Reference

#### - 강의자료만을 참고


