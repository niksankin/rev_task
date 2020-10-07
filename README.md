# ������� �1 �� ����� "������ �������� � ������" 
�������� ������� ������ ���161 ������ �.�.

## �������� ���������
��������� ����������� � ������������ ��������� ������ � ������������ ������ � 256 ���� (������ ��� ���� ������� ����������).
����� ��������� ��������������� �������� ������, ������ ��������� �� �����. ��� ������� ������� �� ������������ ��������� �� 
�������������� ���������� ���������� ���������� ��������� ����� �������� ������, ������� ���� ������������ ��� ���������
������� ����������. ��� ���������� ������ ��������� ����������������� �������� ������������� �����, ���������� �������� �����������.

## ������ ������ ���� �� �������
### ����������� ���� ��������� �� ����� ����������
������ ����� ��������� �������� �������� ��������� �� ����� ����������, ��� ���������� ����������� ������ (��� � ���������������� �����
���������� �� ����, �� ����� ���� ����������� ����������). �� ��������� ������� � ����� ��������� ����� ����� ������� "read-only".
����� ���� ������� � ������� ������ �� "read-write" ����� ��������� ��� ������ ������ ������� `VirtualProtect` �� 
������� WinAPI � ��������� ����� `PAGE_EXECUTE_READWRITE`. ����� ������ � ������ ������ ������� ����� ����������� ��� ��,
��� � � ������� ������� �� �����.
### Vectored Exception Handling (VEH)
� Windows ������� ����������� ����������� ���������������� ������������ ����������. ���������� ������ ����������� �����
��������� ��� ������ ������ ������� `AddVectoredExceptionHandler`. ������ ����� ������������ �� ���, ��� ��� ����������
���� ���������� ��� ���������� � ��������� ����� ��������������� � �������������� ����������, ���� ���� �� ���������� �� ���������
���� ���������. ����� �������, �������� ������� ���������������� ����������� ���������� �� �����������, � ���������� ��������� ��� ����������
����� ���������� �� ���������� ��� ���������.
### �����������/���������� ����������� ����� �� ���� ���������
������ ����� ��������� �������������� ����������� ���������. �� ����� ���������� ������ ����� ����� �������� ������� � ������, ����
����� �������� ����������� �� ��������������� ����� ���� �� ���������� �������� �����������.
### ���������� ���� ��� ������������� ����������� ����������
� �++, ����� ������������� ����������� �������� � ������������� ����������� ���������� ���������� �� ���������� ���������
���� ��������� (������� `main()`). ��� ��������� ������ ����� ������������ ����, ��������� ��������� ������ ������ �������, 
����������� � startup ���� ���������.
### ������ ������� ������� �����������
* ������������ ������� `__declspec(naked)` - ���������� ����������� ��� ������� ��� ������� � �������. 
����� ��������� ������������ ������, ��� ������� �������� ������� ����, � �� ����������� �����. ��������� ������������
������� ��������� �������� ������� �������.
* `IsDebuggerPresent` - ������ ������� ��������� ������� ����� `BeingDebugged` � ��������� `PEB` (Process Environment Block)
��������, ��� ��������� ���������� ��������� ����������������� ������. ��� ��� ����� `IsDebuggerPresent` ����� ���������������
��������� ���������� ��� ����� ���������� � ������� ������� ���������, ����� �������� ����� ���� ������� �� �������������
�� ����������� ���:
    ```
    mov eax, fs:0x18		
    mov eax, [eax + 0x30]
    mov al, [eax + 2]
    ```
    � ��������� ��������� �������� � �������� `al` �������.
* ������������� `jmp` �� �������� �������� - ����� ��������� �������� ��������, ��� `jmp` �������� ������ �������.
* ������������� ������������� ��������� �������� ���� �� ����� (������������� ���� `nies[]{}`) - �������� ������ � ���
�������, � �� � ������ ������. ��� ����� ��������� �������� ������ �� ������� � ����� (�� �� ������ �� ������ � ����).
* "��������" � ��������������� ��� - ������ ������� ���� ����������. � ���������� � ������� �������� ����� ���������
������������ ���������� ����� ���� � ���� �������� ������ ����
## �������� ������ ���������
### �������� ������� ��� �������
���������� ������ ����������� �������� `generate_flag()`. ��� ����������� ������ � ������������, � ����� ����������� �� ���
������ ���������� �������� `xor`. �������� ������ �������, ����������� `xor` �� ��������� `0x94`, �������� �������� ���������.
### ���� ���������� �� ������� main()
��������� ������ ������� `generate_flag()` ����� ��������� ������ ��� ������� ���������; �� ����������� �������� ���������� ��� ��������� ��������
������ ������������ ������ `Hidden`:
```
Hidden()
hidden_prepare_code()
hidden_prepare_data()
hidden_add_veh()
```
������ ������ ����� ��������� �� ������ `main()` � ���������� ���� �������. ����������� `Hidden()` ������������� �����
`read-write` �� ������ ������ ������ ���� `generate_flag()`, ����� ���� ����� `hidden_prepare_code()` �������� ����������
```
mov bl, [eax]
xor bl, 0x94
```
�� ���
```
mov bl, [eax+1]
xor bl, [eax]
```
, � ����� �������� ���������� `xor eax, fs:[0]` �� `xor eax, ds:[0]`, ��� ��������� ����� �������� ����������� ����������.
����� �������, �� ���������� ��������� �������� ��� ����������� ������� ��������� ���������� � ������ ����������, ������� ����� ����������
���������������� ������������ `vectored_handler()`, �������������� ������ ������� `hidden_add_veh()`. ���������� �������
`hidden_prepare_data()` ���������� ��������� ������ `flag` ������� ����� � �������������� ������ ������ � ����� ����������� ����.
��� ������ ����� ������������ ������ ����������� `vectored_handler()`.

�����, ���� ���������� ���� �� ������� `main()` ��������� ��������� ��������� � ��� ��������� �����:
* � �������� ������ ������� `flag` ������� �� �������� ������ �����, � ������, ��� �������� �������� ���������
�������� `xor` � ������� ������� ����������� �����
* ������ ���������� ��� ������� ������� `flag` �������� `xor` �� ��������� `0x94`, ����������� `xor` ������� ����� �� `flag` 
�� ��������� �� ��� �� ������� ������
* ������ ����������, �� ���������� ����������� ����������, ������������� ������ ���������� ���������� ����������, ������
������������ ����������������

��������� ��� �������� � ������������� ���������� � �������� ���������� ����������� `vectored_handler()`.

### ���� ���������� ����������� VEH
��� ����������� `vectored_handler()` ���������� ��������� �������� `hidden_prepare_data()` ������ ������ � ����� ����������� ���� ���
���������� �� ���� ��� ����� SHA256. ������ ���-����� ������������ ��� ���� ��� ��������� rc4, ������� ����� ��������� ���������� ������� `flag`.
������ �� ������������� ������� ����� ������������ ������������.

## ������
��� ������ ��������� ������� ���� ������� `crack.sln` � ������� ������� � ������� Visual Studio (v142) � ������������ `Debug-x86`