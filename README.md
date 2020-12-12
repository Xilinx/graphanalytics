# High Level Architecture
## General use cases
### Synthea use case
Health and medical professionals are required to make predictions for diagnosis and prognosis in various medical problems. Individualized predictive analytics based on patient similarity is becoming popular in this field to identify similar patients to an index patient, derive insights from the records of the similar patients, and provide personalized predictions. Studies have shown that Consine similarity metrics can outperform conventional predictive modeling in which all available patient data are analyzed. The improved prediction performance is achieved at the cost of increased computational burden. 

Xilinx Graph Analytics library optimized for [Alveo Adaptable Accelerator Cards](https://www.xilinx.com/products/boards-and-kits/alveo.html) has been demonstrated on [Synthea<sup>TM</sup>](https://synthetichealth.github.io/synthea/) generated patient data to greatly reduce that computational burden and provide predication results for patients more rapidly. Synthea<sup>TM</sup> is an open-source, synthetic patient generator that models the medical history of synthetic patients and provides high-quality, synthetic, realistic but not real, patient data and associated health records covering every aspect of healthcare. 
 
### Product recommendation (UHG owns), 
### Healthcare (UHG owns), 
### Helpdesk (UHG owns), 
### Log file errors (UHG Owns), 
### Security (UHG owns) etc

## Xilinx Vitis Graph Analytics Acceleration Library Plugin for TigerGraph
Xilinx Vitis Graph Analytics Acceleration Libray is seamlessly integrated with TigerGraph as a plugin. The low level hardware details are transparent to TigerGraph users.

The plugin consists of two major components as shown in the block diagram below:
* Xilinx Vitis Graph Analytics Library is provided as custom TigerGraph User Defined Function(UDF) that can be called directly from GSQL. 
* Xilinx Resource Manager (XRM) and Xilinx Run Time (XRT) libraries are installed on all processing nodes to manage resource allocation on Alveo Acceleration cards and data movement between the CPU and the FPGA.
<p align="center">
<img src="docs/images/xilinx-tg-plugin.png"  width="400">
</p>


## Cosine similarity
Cosine similarity is a measure of similarity between two non-zero vectors of an inner product space: 
<p align="center">
<img src="https://wikimedia.org/api/rest_v1/media/math/render/svg/1d94e5903f7936d3c131e040ef2c51b473dd071d">
</p>
In TigerGraph patients information and medical history are represented as vertexes and edges in a graph database as shown below:  
<p align="center">
<img src="https://media.gitenterprise.xilinx.com/user/1683/files/94772d00-37a2-11eb-8687-f3bbed4892b5" width="500">
</p>

Each attribute (e.g. age, gender, race, immunization, etc) of a patient is mapped to a numerical value and becomes a feature of the patient vector. In our PoC design each paitent vector consists of 198 features and each feature is stored as a 32-bit integer. Consine similarity is calcualted between the index patient vector and all other patient vectors. The results are then sorted and the top 100 patients with highest scores are presented. Below is the detail of the feature map:

| Feature (INT)  | Descriptions       | 
| :------------- |:-------------------|
| 0              | Norm               |
| 1-2            | 64-bit patient VID |
| 3              | age                |
| 4              | gender             | 
| 5              | race               |
| 6              | ethnicity          | 
| 7-19           | reserved           |
| 20-39          | immunization map   | 
| 40-59          | allergy map        |
| 60-109         | conditions map     |
| 110-139        | imaging studies map|
| 140-189        | procedures map     | 
| 190-209        | careplans map      |

### Cosine similarity GSQL
TigerGraph uses GSQL query language for fast and scalable graph operations and analytics. A reference design of patient similarity based on cosine similarity was created in GSQL to baseline the functionalities and computation complexity of the algorithm. It is used to verify and validate the functioanl correctness and performance improvement of the Alveo accelerated desgin. Below is the block diagram of cosine similarity computation in GSQL, which is executed entirely on CPU.
<p align="center">
<img src="https://media.gitenterprise.xilinx.com/user/1683/files/2f452b00-37e6-11eb-9dde-0cd66e22b7d8"  width="400">
</p>

### FPGA Accelerated cosine similarity function
Xilinx Vitis Graph Analytics Library plugin provides a user defined function that offloads the computation of cosine simimarity and the top K highest scores to the FPGA:
<p align="center">
<img src="https://media.gitenterprise.xilinx.com/user/1683/files/33c41080-37f3-11eb-92ad-6e36f364849a" width="400">
</p>

The accelerated cosine simlarity UDF consists of two parts:
* Host code: C++ code that runs on the CPU to manage resource allocation and data movement between the CPU and the FPGA
* Kernel: custom computation hardware logic that utilizes massive parallel processing horsepwer and abundant on-chip memory on FPGA. Each kernel contains two compute units(CUs) running in parallel with each CU connecting to one HBM stack that stores patients' records.

<p align="center">
<img src="https://media.gitenterprise.xilinx.com/user/1683/files/db8f0d80-37f6-11eb-9c4b-609d4932be72" width="400">
</p>

The kernel design for each CU is illustrated in the block diagram below. Each CU contains 16 fully pipelined cosine similarity processing elements (PEs) and one MaxK components to choose the top similarities. The 16 PEs are connected to 16 channels to access 5M patients' data in parallel. The incoming new patient's record is transmitted to the FPGA's PLRAM by the host and then duplicated to 16 PEs. The MaxK primitive calculates the top cosine similarities and their corresponding indices and writes them to the PLRAMA, which is read out by the host. In the end, the host will do a simple computation to extract the final top similarities from the two returned top similarity sets computed by the two CUs.
<p align="center">
<img src="https://media.gitenterprise.xilinx.com/user/1683/files/468d1400-37f8-11eb-90f1-5bf506ecf643" width="400">
</p>

### Test drive cosine similarity acceleration on Alveo U50 on premise

### Test drive cosine similarity acceleration on Azure NP
NP Azure Virtual Machines for HPC coming soon – Our Alveo U250 FPGA-Accelerated VMs offer from 1-to-4 Xilinx U250 FPGA devices as an Azure VM- backed by powerful Xeon Platinum CPU cores, and fast NVMe-based storage. The NP series will enable true lift-and-shift and single-target development of FPGA applications for a general purpose cloud. Based on a board and software ecosystem customers can buy today, RTL and high-level language designs targeted at Xilinx’s U250 card and SDAccel 2019.1 runtime will run on Azure VMs just as they do on-premises and on the edge, enabling the bleeding edge of accelerator development to harness the power of the cloud without additional development costs.

# References
* Lee J, Maslove DM, Dubin JA. Personalized mortality prediction driven by electronic medical data and a patient similarity metric. PLoS One 2015 May;10(5):e0127428 [FREE Full text] [CrossRef] [Medline]
