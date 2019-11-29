#include "Enclave_u.h"
#include <errno.h>

typedef struct ms_LEA_test_t {
	unsigned char* ms_key;
	size_t ms_key_len;
	unsigned char* ms_text;
	size_t ms_text_len;
} ms_LEA_test_t;

typedef struct ms_ocall_print_string_t {
	const char* ms_str;
} ms_ocall_print_string_t;

static sgx_status_t SGX_CDECL Enclave_ocall_print_string(void* pms)
{
	ms_ocall_print_string_t* ms = SGX_CAST(ms_ocall_print_string_t*, pms);
	ocall_print_string(ms->ms_str);

	return SGX_SUCCESS;
}

static const struct {
	size_t nr_ocall;
	void * table[1];
} ocall_table_Enclave = {
	1,
	{
		(void*)Enclave_ocall_print_string,
	}
};
sgx_status_t printf_helloworld(sgx_enclave_id_t eid)
{
	sgx_status_t status;
	status = sgx_ecall(eid, 0, &ocall_table_Enclave, NULL);
	return status;
}

sgx_status_t LEA_test(sgx_enclave_id_t eid, unsigned char* key, unsigned char* text)
{
	sgx_status_t status;
	ms_LEA_test_t ms;
	ms.ms_key = key;
	ms.ms_key_len = key ? strlen(key) + 1 : 0;
	ms.ms_text = text;
	ms.ms_text_len = text ? strlen(text) + 1 : 0;
	status = sgx_ecall(eid, 1, &ocall_table_Enclave, &ms);
	return status;
}

