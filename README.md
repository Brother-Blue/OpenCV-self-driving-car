# 2021-group-01 
[![pipeline status](https://https://git.chalmers.se/courses/dit638/students/2021-group-01/badges/master/pipeline.svg)](https://git.chalmers.se/courses/dit638/students/2021-group-01/commits/master)
[![pipeline status](https://git.chalmers.se/courses/dit638/students/2021-group-01/badges/master/pipeline.svg)](https://git.chalmers.se/courses/dit638/students/2021-group-01/-/commits/master)
## Getting started
1. Set up your GitLab account's [SSH key](https://git.chalmers.se/profile/keys) if you already have a SSH key, skip to #2.
    - Open the terminal and enter the command: ssh-keygen -t rsa -b 2048 -C "optional comment"
    - Locate the key in the specified path
    - Open and copy the contents of the id_key.pub file, and paste it into the textbox in the link above.
    - Title your key, and click "Add key"
    ---
2. Navigate or create a directory that you would like to store the project in.
    - Once completed, navigate to the project page and copy the "Clone with SSH" link 
        - git@git.chalmers.se:courses/dit638/students/2021-group-01.git
    - Open a new terminal and navigate to the directory
    - Clone the project's contents with: git clone link_from_above
    ---
3. Set up Docker, if you already have Docker installed skip to #4
    ```shell
    sudo apt-get update
    ```
    <br>Setup the Docker repository
    ```shell
    sudo apt-get install \
        <br>apt-transport-https \
        <br>ca-certificates \
        <br>curl \
        <br>gnupg \
        <br>lsb-release
    ```

    Add Docker's GPG key
    ```shell
    curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg

    echo \
    "deb [arch=amd64 signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] https://download.docker.com/linux/ubuntu \
    $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
    ```

    Install the Docker engine
    ```shell
    sudo apt-get install docker-ce docker-ce-cli containerd.io
    ```
    ---

4. Run the Docker image
    - cd to the cloned repository
    - Run the following commands, replacing <code><your_id></code> with your respecitve identifier
    ```docker
    docker run --rm -ti -v $PWD:/opt/sources ubuntu:18.04 /bin/bash
    docker run --rm <your_id>/container_name
    ```

## Our way of working

### Adding features
- Discuss and Define new feature
- Create appropriate requirements
- Add requirements to Kanban board
- Implement feature that satisfies the requirements
- Run extensive testing
### Updating features
- Discuss feature changes
- Create and/or update appropriate requirements 
- Add/update requirements to Kanban board 
- Implement feature that satisfies the requirements 
- Run extensive testing
### Maintaining features 
- Discover bug(s) from unit testing 
- Create issues       
- Add issues as items on Kanban board                   
- Implement changes 
- Run extensive testing 
- Repeat cycle if needed

## Commit Structure
Imperative subject line (max 50 chars)<br>
<br>
Commit body. (max line length 72 chars)<br>
What changed.<br>
Why was it changed.<br>

Example:
```
Update a faulty library

Updated library A to library B.
Library A contained a method that would immediately raise an exception
regardless of its implementation. Library B is an updated version of
library A which fixes this issue.
```
