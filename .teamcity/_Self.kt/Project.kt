// Copyright © 2026 CCP ehf.

package _Self

import _Self.buildTypes.*
import jetbrains.buildServer.configs.kotlin.*
import jetbrains.buildServer.configs.kotlin.Project
import jetbrains.buildServer.configs.kotlin.vcs.GitVcsRoot


object Project : Project({

    description = "Build / Publish pipeline for https://github.com/carbonengine/imagetools"

    params {
        param("carbon_ref", "refs/heads/main")
        param("carbon-pipeline-tools-ref", "refs/tags/v0.1.0")
    }
    
    subProject(Windows.Project)

    buildType(PublishToPerforce)
    buildType(SyncToMirror)
})
