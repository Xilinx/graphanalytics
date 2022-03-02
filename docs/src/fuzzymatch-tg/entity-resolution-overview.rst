===================================
Entity Resolution Use Case Overview
===================================

In this use case, two data sources are used. The first data source is a reference 
CSV file with a list of last names. The second data source is a new CSV file 
with a new list of last names that slightly differ from the reference. The table
below shows some examples:

.. list-table:: 
   :widths: 15 15 70
   :header-rows: 1

   * - Reference Name
     - New Name
     - Levenshtein Edit Distance
   * - HIGGINBOTHAM
     - HJGGJNCOTHAM
     - 3
   * - STRINGFELLOW
     - STRINGFELMOW
     - 1
   * - VANLANDINGHAM
     - VAOLBNDINGHAM
     - 2

The reference names are loaded to the Xilinx Alveo card first. The list of new 
names will be sent to the card to match against the reference with a user sepcified
threshold. If the Levenshtein edit distance between the new name and the reference
is within the threshold, a match is found.

