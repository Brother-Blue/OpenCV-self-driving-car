# 2021-group-01

## Getting started
1. Set up your GitLab account's [SSH key](https://git.chalmers.se/profile/keys) if you already have a SSH key, skip to #2.
    - Open the terminal and enter the command: ssh-keygen -t rsa -b 2048 -C "optional comment"
    - Locate the key in the specified path
    - Open and copy the contents of the id_key.pub file, and paste it into the textbox in the link above.
    - Title your key, and click "Add key"

2. Navigate or create a directory that you would like to store the project in.
    - Once completed, navigate to the project page and copy the "Clone with SSH" link 
        - git@git.chalmers.se:courses/dit638/students/2021-group-01.git
    - Open a new terminal and navigate to the directory
    - Clone the project's contents with: git clone link_from_above

## Our way of working

### Adding features
- To be added
### Updating features
- To be added
### Maintaining features
- To be added

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
