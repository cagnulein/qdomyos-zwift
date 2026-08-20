PATCH_JOB = $$(GITHUB_JOB)
equals(PATCH_JOB, linux-x86-build) {
    system(python3 $$PWD/oneoff_patch_4961.py)
}

include(qdomyos-zwift.pri)

QMAKE_IOS_DEPLOYMENT_TARGET = 12.0
QMAKE_DEVELOPMENT_TEAM = 6335M7T29D
QMAKE_CODE_SIGN_IDENTITY = "iPhone Developer"
QMAKE_CODE_SIGN_STYLE = Automatic