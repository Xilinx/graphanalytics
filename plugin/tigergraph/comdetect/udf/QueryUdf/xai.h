/* Define input data type, result data type, id type and number of channels */


/* Data type of the inputs */
#define SPATIAL_dataType int

/* Data type of the result */
#define SPATIAL_resDataType double

/* Data type of the id used for identifying each data. Example: Graph Node VID can be used as id whose type is uint64_t */
#define SPATIAL_idType uint64_t

/* Number of FPGA devices. Example: Each U50 card has one device. So, 3 U50 cards will have 3 FPGA devices */
#define SPATIAL_numDevices 3

/* Number of compute units (kernel instances) on each FPGA device */
#define SPATIAL_numComputUnits 2

/* Number of parallel channels on each compute unit */
#define SPATIAL_numChannels 16

extern "C" {
  /* A pointer to any object that needs to be passed around. Object can be a C++
  object. The host code library can cast it to right object based upon context
  and have access to its fields. The client can just hold on to it and passes it
  back to the host library for providing the right data to operate on.
  As per need, client can include a separate header .hpp file provided by the
  host library to gain access to specific fields.
  */
  typedef void* xaiHandle;

  typedef enum e_xai_algorithm {
    xai_algo_cosinesim_ss,
    xai_algo_jackardsim_ss,
    xai_algo_knearestneighbor,
    xai_algo_louvain_modularity
  } t_xai_algorithm;

  /* Struct type to hold id and result */
  typedef struct s_xai_id_value_pair {
    SPATIAL_idType id;
    SPATIAL_resDataType value;
  } t_xai_id_value_pair, *p_xai_id_value_pair;

  /* Struct type to hold all the necessary state of one invocation instance */

  typedef struct s_xai_context{
    const char*  xclbin_filename; // Name of the xilinx xclbin file name
    unsigned int num_devices;     // Use specified number of devices if available
    unsigned int vector_length;   // Number of elements in each vector/record: l_n
    unsigned int num_result;      // Number of result elements: l_k
    unsigned int num_CUs;         // Number of CUs in xclbin
    unsigned int num_Channels;    // Number of channels in xclbin
    unsigned int start_index;     // Starting index of compute elements in the vector/record
    unsigned int element_size;    // Number of bytes needed to store one element
  } t_xai_context, *p_xai_context;


  /* Function pointer types for the interface functions */
  typedef xaiHandle (*t_fp_xai_open)(p_xai_context);
  typedef int (*t_fp_xai_cache_reset)(xaiHandle);
  typedef int (*t_fp_xai_cache)(xaiHandle, SPATIAL_dataType*,unsigned int);
  typedef int (*t_fp_xai_cache_flush)(xaiHandle);
  typedef xaiHandle (*t_fp_xai_execute)(xaiHandle, t_xai_algorithm, SPATIAL_dataType*, p_xai_id_value_pair);
  typedef int (*t_fp_xai_close)(xaiHandle xaiInstance);
  typedef int (*t_fp_xai_create_partitions)(int argc, char** argv);
  typedef int (*t_fp_xai_load_partitions)(int argc, char** argv);
  typedef int (*t_fp_xai_execute_louvain)(int argc, char** argv);


  /* Open Device, Allocate Buffer, Doanload FPGA bit stream and return an object
  as xaihandle that captures full state of this particualar instance of
  opening */
  xaiHandle xai_open(p_xai_context context);

  /* Reset writing to beginning of the arg_Y buffer */
  int xai_cache_reset(xaiHandle xaiInstance);

  /* Incrementally buffer for arg_Y in the x86 attached memory */
  int xai_cache(xaiHandle xaiInstance, SPATIAL_dataType* populationVec, unsigned int numElements);

  /* Write input arg arg_Y to alveo attached memory. */
  int xai_cache_flush(xaiHandle xaiInstance);

  /* Run algorith "algo" on data already stored on alveo memory using xai_write and data passed
  using arg_X and return result as an array of s_xai_id_value_pair
  */
  int xai_execute(xaiHandle xaiInstance, t_xai_algorithm algo, SPATIAL_dataType* arg_X, p_xai_id_value_pair result);

  /* Free up memory and close device */
  int xai_close(xaiHandle xaiInstance);

  /* Create partitions */
  int xai_create_partitions(int argc, char** argv);

  /* Load partitions to FPGAs */
  int xai_load_partitions(int argc, char** argv);

  /* Compute Louvain Modularity */
  int xai_execute_louvain(int argc, char** argv);

}
