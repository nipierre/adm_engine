
# ADM Engine AMQP worker

The purpose of this worker is to integrate the `adm_engine` tool easily into a message broker environment.
It is based on the [c_amqp_worker](https://github.com/media-cloud-ai/c_amqp_worker) library.
The worker can handle AMQP message under JSON format. Here are some usage examples:

 * Dumping BW64/ADM file info:
    ```json
    {
      "job_id": 123,
      "parameters": [
        {
          "id": "input",
          "type": "string",
          "value": "/path/to/bw64_adm.wav"
        }
      ]
    }
    ```


 * Rendering ADM:
    ```json
    {
      "job_id": 123,
      "parameters": [
        {
          "id": "input",
          "type": "string",
          "value": "/path/to/bw64_adm.wav"
        },
        {
          "id": "output",
          "type": "string",
          "value": "/path/to/output/directory"
        }
      ]
    }
    ```


 * Rendering specified ADM element:
    ```json
    {
      "job_id": 123,
      "parameters": [
        {
          "id": "input",
          "type": "string",
          "value": "/path/to/bw64_adm.wav"
        },
        {
          "id": "output",
          "type": "string",
          "value": "/path/to/output/directory"
        },
        {
          "id": "element_id",
          "type": "string",
          "value": "APR_1002"
        }
      ]
    }
    ```


 * Rendering ADM, applying gains to elements:
    ```json
    {
      "job_id": 123,
      "parameters": [
        {
          "id": "input",
          "type": "string",
          "value": "/path/to/bw64_adm.wav"
        },
        {
          "id": "output",
          "type": "string",
          "value": "/path/to/output/directory"
        },
        {
          "id": "gain_mapping",
          "type": "array_of_strings",
          "value": [
            "AO_1001=-4.0"
          ]
        }
      ]
    }
    ```


 * Rendering a specified ADM element, applying gains on others:
    ```json
    {
      "job_id": 123,
      "parameters": [
        {
          "id": "input",
          "type": "string",
          "value": "/path/to/bw64_adm.wav"
        },
        {
          "id": "output",
          "type": "string",
          "value": "/path/to/output/directory"
        },
        {
          "id": "element_id",
          "type": "string",
          "value": "APR_1002"
        },
        {
          "id": "gain_mapping",
          "type": "array_of_strings",
          "value": [
            "ACO_1003=3.0",
            "ACO_1004=-6.0"
          ]
        }
      ]
    }
    ```

