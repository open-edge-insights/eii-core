# Contributing to IEdgeInsights

## Merge Requests

Everybody can propose a merge request (MR) but only the
core-maintainers of the project can merge it.

### Commit and MR Hygiene

The following points will be especially looked at during the review.

1. **master** is the default branch, it is advised always to work on a new
   feature/bug fix developer branch by keeping your local **master** branch
   untouched. The below convention while naming our branches can be followed:
   * Feature branches - feature/idsid/feature_name
   * Bugfix branch - bugfix/idsid/jira_bug_ids

   More details on best git branching model
   [https://nvie.com/posts/a-successful-git-branching-model/](https://nvie.com/posts/a-successful-git-branching-model/)

2. Once your branch is created by following the above convention
   (`git checkout -b <branch_name>`) and it's ready with the changes,
   run the below commands:
   * `git commit -s` - commit the message just like the old fashioned
     way with one liner and the body. Commit message
     format below:

     ```sh
      <module_name>: one liner about the commit

      Additional details about the commit message

      Signed-off by: abc <abc@intel.com>
     ```

     Here, `module_name` loosely refers to the folder name where the major
     change is been introduced.

   * `git push origin <branch_name>` - pushes your changes to the remote
     branch name (orgin/<branch_name>)
   * Create a merge request in gitlab
   * If one notices any conflicts after creating a pull request, just
     apply the latest master to your <branch_name> by running commands:
      * `git checkout master` - with your current branch being <branch_name>
      * `git pull` - pull latest remote master commits to your local master
        branch
      * `git checkout <branch_name>` - Get back to your branch
      * `git rebase master` - rebase your branch with latest master

3. After addressing code review comments, do a `git commit --amend` to amend
   the commit message and do a `git push -f origin <branch_name>` to forcely
   push your changes. This is needed as we want to maintain a single commit.

### Minimum requirements for a MR

The following are the minimal requirements that every MR should meet.

- **Pass Continuous Integration (pre-merge build)**: Every MR has to pass
  our CI. Below are the scripts in [CITests repo]() that would be invoked
  by the pre-merge build:
  1. `pre_recieve_hook.sh` - does linting for changed go and python files
      w.r.t latest master
  2. `pre_merge_and_nightly_build_script.sh` - builds and runs the pre-merge
      build
  3. We are in the process of adding the `pre-merge` testsuite to perform
     santiy testing

### Review Process

The reviewers may be busy. If they take especially long to react, feel free to
trigger them by additional comments in the MR thread.

It is the job of the developer that posts the MR to rebase the MR on the target
branch when the two diverge.

Below are some additional stuff that developers should adhere to:

* Since there is no batch feature available in the currently deployed IT gitlab
  version, let’s not bloat the reviewers inboxes with “done” messages. If one is
  done addressing comments and pushing the changes, he/she can leave a comment
  saying that all the above review comments have been addressed. Only for those
  comments, where in we need extra clarifications, we can have discussion on
  them.

* It’s been noticed when we merge the merge-request with squash and merge
  feature especially when there are multiple commits for the same change, gitlab
  only picks up the merge-request title and doesn’t pick up the description
  added in the merge-request description and doesn’t give a preview of how the
  commit message would look like at the time of merging
  (More details at https://gitlab.com/gitlab-org/gitlab-ce/issues/49803) which
  is very bad as we developers add useful definition there which will not make
  it to commit history.

  To circumvent this, all of us can maintain just a single commit with all the
  additional details in our commit description. Luckily unlike github, you can
  compare the force pushed changes with the previous version in gitlab which
  will be very helpful for reviewers.

*	Whenever one sees any failure in the pre-merge build, just check if “rebasing”
  your merge-request in the gitlab UI can solve the issue (there will be rebase
  button). If not, please apply access to CI environment to see the reason
  behind failure.
