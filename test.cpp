#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <math.h>

#define LEN(arr) (sizeof(arr) / sizeof(arr[0]))
const char *getErrorString(cl_int error)
{
	switch (error) {
		// run-time and JIT compiler errors
	case 0: return "CL_SUCCESS";
	case -1: return "CL_DEVICE_NOT_FOUND";
	case -2: return "CL_DEVICE_NOT_AVAILABLE";
	case -3: return "CL_COMPILER_NOT_AVAILABLE";
	case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
	case -5: return "CL_OUT_OF_RESOURCES";
	case -6: return "CL_OUT_OF_HOST_MEMORY";
	case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
	case -8: return "CL_MEM_COPY_OVERLAP";
	case -9: return "CL_IMAGE_FORMAT_MISMATCH";
	case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
	case -11: return "CL_BUILD_PROGRAM_FAILURE";
	case -12: return "CL_MAP_FAILURE";
	case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
	case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
	case -15: return "CL_COMPILE_PROGRAM_FAILURE";
	case -16: return "CL_LINKER_NOT_AVAILABLE";
	case -17: return "CL_LINK_PROGRAM_FAILURE";
	case -18: return "CL_DEVICE_PARTITION_FAILED";
	case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

		// compile-time errors
	case -30: return "CL_INVALID_VALUE";
	case -31: return "CL_INVALID_DEVICE_TYPE";
	case -32: return "CL_INVALID_PLATFORM";
	case -33: return "CL_INVALID_DEVICE";
	case -34: return "CL_INVALID_CONTEXT";
	case -35: return "CL_INVALID_QUEUE_PROPERTIES";
	case -36: return "CL_INVALID_COMMAND_QUEUE";
	case -37: return "CL_INVALID_HOST_PTR";
	case -38: return "CL_INVALID_MEM_OBJECT";
	case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
	case -40: return "CL_INVALID_IMAGE_SIZE";
	case -41: return "CL_INVALID_SAMPLER";
	case -42: return "CL_INVALID_BINARY";
	case -43: return "CL_INVALID_BUILD_OPTIONS";
	case -44: return "CL_INVALID_PROGRAM";
	case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
	case -46: return "CL_INVALID_KERNEL_NAME";
	case -47: return "CL_INVALID_KERNEL_DEFINITION";
	case -48: return "CL_INVALID_KERNEL";
	case -49: return "CL_INVALID_ARG_INDEX";
	case -50: return "CL_INVALID_ARG_VALUE";
	case -51: return "CL_INVALID_ARG_SIZE";
	case -52: return "CL_INVALID_KERNEL_ARGS";
	case -53: return "CL_INVALID_WORK_DIMENSION";
	case -54: return "CL_INVALID_WORK_GROUP_SIZE";
	case -55: return "CL_INVALID_WORK_ITEM_SIZE";
	case -56: return "CL_INVALID_GLOBAL_OFFSET";
	case -57: return "CL_INVALID_EVENT_WAIT_LIST";
	case -58: return "CL_INVALID_EVENT";
	case -59: return "CL_INVALID_OPERATION";
	case -60: return "CL_INVALID_GL_OBJECT";
	case -61: return "CL_INVALID_BUFFER_SIZE";
	case -62: return "CL_INVALID_MIP_LEVEL";
	case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
	case -64: return "CL_INVALID_PROPERTY";
	case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
	case -66: return "CL_INVALID_COMPILER_OPTIONS";
	case -67: return "CL_INVALID_LINKER_OPTIONS";
	case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

		// extension errors
	case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
	case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
	case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
	case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
	case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
	case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
	default: return "Unknown OpenCL error";
	}
}
//kernel source code
const char* src[] = {
	"__kernel void vec_add(__global const float *a, __global const float *b, __global float *c)\n",
	"{\n",
	"	int gid = get_global_id(0);\n",
	"	c[gid] = a[gid] + b[gid];\n",
	"}\n"
};
const long long int ARRAY_SIZE = 20000000;
float srcA[ARRAY_SIZE];
float srcB[ARRAY_SIZE];
float dst[ARRAY_SIZE];

int main()
{

	cl_uint numPlatforms;
	cl_platform_id platform = NULL;
	cl_int status = clGetPlatformIDs(0, NULL, &numPlatforms);
	if(status != CL_SUCCESS)
	{
		printf("error: getting platforms/n");
		exit(1);
	}
	cl_platform_id *platforms = (cl_platform_id *)malloc(numPlatforms*sizeof(cl_platform_id));
	status = clGetPlatformIDs(numPlatforms, platforms, NULL);
	if(status != CL_SUCCESS)
	{
		printf("error: getting platform IDs/n");
		exit(1);
	}
	platform = platforms[0];
	cl_context_properties cps[3] ={
	CL_CONTEXT_PLATFORM,
	(cl_context_properties) platform, 0};
	cl_context_properties *cprops =
	(NULL == platform) ? NULL :cps;
	//create OpenCL context
	cl_context context = clCreateContextFromType(cprops, CL_DEVICE_TYPE_GPU, NULL, NULL, &status);
	if(status != CL_SUCCESS)
	{
		printf("create context error/n");
		exit(1);
	}
	
	//get device id from context
	size_t cb;
	status = clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &cb);
	if(status != CL_SUCCESS)
	{
		printf("get context info 1 error/n");
		exit(1);
	}
	cl_device_id *devices= new cl_device_id[cb / sizeof(cl_device_id)];
	status = clGetContextInfo(context, CL_CONTEXT_DEVICES, cb, devices, NULL);
	if(status != CL_SUCCESS)
	{
		printf("get context info 2 error/n");
		exit(1);
	}
	//create a command queue
	cl_command_queue cmd_queue = clCreateCommandQueue(context, devices[0], 0, NULL);

	//create kernel
	cl_program program = clCreateProgramWithSource(context, LEN(src),src, NULL, NULL);
	status = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	 if(status!=CL_SUCCESS){
        printf("vlBuildProgram is %d---->%s\n",status,getErrorString(status));
        char tbuf[0x10000];
        clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, 0x10000,tbuf,NULL);
        printf("build info is %s\n",tbuf);
    }
	cl_kernel kernel = clCreateKernel(program, "vec_add", NULL);

	//host initialize
	for (int i = 0; i < ARRAY_SIZE; i++)
	{
		srcA[i] = (float)i*acos(-1);
		srcB[i] = (float)(ARRAY_SIZE - i)*acos(-1);
	}
	
	//set the input and output arguments of kernel
	cl_mem memobjs[3];
	memobjs[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float)*ARRAY_SIZE, srcA, NULL);
	memobjs[1] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float)*ARRAY_SIZE, srcB, NULL);
	memobjs[2] = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float)*ARRAY_SIZE, NULL, NULL);

	//set "a", "b", "c" vector argument
	status = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&memobjs[0]);
	status |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&memobjs[1]);
	status |= clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&memobjs[2]);

	size_t global_work_size[1] = {ARRAY_SIZE};

	//execute kernel
	status = clEnqueueNDRangeKernel(cmd_queue, kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);

	//read output array
	status = clEnqueueReadBuffer(cmd_queue, memobjs[2], CL_TRUE, 0, ARRAY_SIZE*sizeof(cl_float), dst, 0, NULL, NULL);

	for(int i=0; i<5; ++i)
	{
		printf("-> %.2f\n", dst[i]);
	}
	return 0;
}
	
