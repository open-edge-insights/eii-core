## Steps to run the generate_multiple_VI_VA_docker-compose.py helper function

1. Open [config.json](config.json) file and update the path of EIS repository
   * for example
   ```
      "eis_path": "Absolute path of IEdgeInsights"
   ```
   to
   ```
      "eis_path": "/home/user/IEdgeInsights"
   ```

2. Command to run the script to generate multiple file (provide the required number as command line argument)
   * for example:
   ```
      python3.6 generate_multiple_VI_VA_docker-compose.py 5
   ```
   
   * To clean the generated files use the following command
   ```
      python3.6 generate_multiple_VI_VA_docker-compose.py clean
   ```